//
// Created by ubuntu on 27.12.16.
//

#include <string>
#include <sstream>

#include "WebService.h"

#include "../JSON/json.hpp"
#include "../Storage/StorageClient.h"

#define HTTP_GET "GET"
#define HTTP_POST "POST"
#define HTTP_PUT "PUT"
#define HTTP_DELETE "DELETE"

#define HTTP_STATUS_SUCCESS 200
#define HTTP_STATUS_CREATED 201
#define HTTP_STATUS_NO_CONTENT 204
#define HTTP_STATUS_BAD_REQUEST 400
#define HTTP_STATUS_NOT_FOUND 404
#define HTTP_STATUS_METHOD_NOT_ALLOWED 405
#define HTTP_STATUS_INTERNAL_ERROR 500


using json = nlohmann::json;

void splitURI(std::string const& uri, std::vector<std::string> tokens) {
    std::string s(uri);
    std::string delimiter = "/";

    size_t pos = 0;
    while ((pos = s.find(delimiter)) != std::string::npos) {
        if (pos > 0) {
            tokens.push_back(s.substr(0, pos));
        }
        s.erase(0, pos + delimiter.length());
    }

    if (s.length() > 0) {
        tokens.push_back(s);
    }
}

std::string generateUUID() {
    CassUuidGen* uuidGen = cass_uuid_gen_new();
    CassUuid uuid;
    cass_uuid_gen_time(uuidGen, &uuid);
    cass_uuid_gen_free(uuidGen);

    char uuid_str[CASS_UUID_STRING_LENGTH];
    cass_uuid_string(uuid, uuid_str);

    return std::string(uuid_str);
}

WebService::WebService() {
    storageClient = new StorageClient("127.0.0.1");
}

WebService::~WebService() {
    delete storageClient;
}

void WebService::web_service_process_request(fastcgi::Request* request) {
    request->setContentType("text/plain");

    auto uri = request->getURI();
    std::vector<std::string> tokens;
    splitURI(uri, tokens);

    auto method = request->getRequestMethod();

    web_service_post_poll(request);
    return;

    size_t tokensCount = tokens.size();
    if (tokensCount == 1) {
        auto& token = tokens.front();
        if (token == "polls") {
            if (method == HTTP_GET) {
                web_service_get_polls(request, nullptr);
            } else if (method == HTTP_POST) {
                web_service_post_poll(request);
            } else {
                request->setStatus(HTTP_STATUS_METHOD_NOT_ALLOWED);
            }
        } else if (token.find("polls?") != std::string::npos) {
            if (request->hasArg("creationDateTime") || request->countArgs() == 1) {
                std::string creationDateTime = request->getArg("creationDateTime");
                web_service_get_polls(request, &creationDateTime);
            } else {
                request->setStatus(HTTP_STATUS_NOT_FOUND);
            }
        } else {
            request->setStatus(HTTP_STATUS_NOT_FOUND);
        }
    } else if (tokensCount == 2) {
        auto& token = tokens.front();
        if (token == "polls" && request->countArgs() == 0) {
            auto& id = tokens.back();
            if (method == HTTP_GET) {
                web_service_get_poll(request, id);
            } else if (method == HTTP_DELETE) {
                web_service_delete_poll(request, id);
            } else {
                request->setStatus(HTTP_STATUS_METHOD_NOT_ALLOWED);
            }
        } else {
            request->setStatus(HTTP_STATUS_NOT_FOUND);
        }
    } else if (tokensCount == 3) {
        auto& token = tokens.front();
        auto& votesToken = tokens.back();
        if (token == "polls" && votesToken == "votes" && request->countArgs() == 0) {
            auto& id = *(tokens.begin() + 1);
            if (method == HTTP_GET) {
                web_service_get_votes(request, id);
            } else if (method == HTTP_POST) {
                web_service_post_vote(request, id);
            } else {
                request->setStatus(HTTP_STATUS_METHOD_NOT_ALLOWED);
            }
        } else {
            request->setStatus(HTTP_STATUS_NOT_FOUND);
        }
    } else if (tokensCount == 4) {
        auto& token = tokens.front();
        auto& votesToken = *(tokens.begin() + 2);
        if (token == "polls" && votesToken == "votes" && request->countArgs() == 0) {
            auto& id = *(tokens.begin() + 1);
            auto& voteId = tokens.back();
            if (method == HTTP_GET) {
                web_service_get_vote(request, voteId, id);
            } else if (method == HTTP_PUT) {
                web_service_put_vote(request, voteId, id);
            } else if (method == HTTP_DELETE) {
                web_service_delete_vote(request, voteId, id);
            } else {
                request->setStatus(HTTP_STATUS_METHOD_NOT_ALLOWED);
            }
        } else {
            request->setStatus(HTTP_STATUS_NOT_FOUND);
        }
    } else {
        request->setStatus(HTTP_STATUS_NOT_FOUND);
    }
}

void WebService::web_service_get_polls(fastcgi::Request* request, std::string const* creationDateTime) {
    const char *message = nullptr;
    std::vector<Poll> polls;

    if (this->storageClient->polls_get(creationDateTime, polls, &message) == QUERY_FAILURE) {
        request->setStatus(HTTP_STATUS_INTERNAL_ERROR);
        if (message) {
            std::stringbuf buffer(message);
            request->write(&buffer);
        }
        return;
    }

    json dataJson;
    std::string* nextCreationDateTime = nullptr;
    for (auto& poll : polls) {
        dataJson.push_back({{ "creationDateTime", poll.creationDateTime },
                            { "name" , poll.name },
                            { "links", { { "self", "/polls/" + poll.getId() } } }
                           });
        nextCreationDateTime = &(poll.creationDateTime);
    }

    json linksJson = { { "self", "/polls" } };
    if (nextCreationDateTime) {
        linksJson.push_back({ "next", "/polls?creationDateTime=" + *nextCreationDateTime });
    }

    json json = { { "data", dataJson }, { "links", linksJson} };

    std::stringbuf buf(json.dump());
    request->write(&buf);
    request->setStatus(HTTP_STATUS_SUCCESS);
}

void WebService::web_service_get_poll(fastcgi::Request* request, std::string const& id) {
    const char *message = nullptr;
    std::vector<Poll> polls;

    if (this->storageClient->poll_get(id, polls, &message) == QUERY_FAILURE) {
        request->setStatus(HTTP_STATUS_INTERNAL_ERROR);
        if (message) {
            std::stringbuf buffer(message);
            request->write(&buffer);
        }
        return;
    }

    if (polls.empty()) {
        request->setStatus(HTTP_STATUS_NOT_FOUND);
    }

    auto poll = polls.front();

    json optionsJson;
    for (auto& option : poll.options) {
        optionsJson.push_back({ { "id", option.id},
                                { "name" , option.name },
                                { "votes", option.votes } });
    }

    json linksJson = { { "self", "/polls/" + id }, { "votes", "/polls/" + id + "/votes" } };
    json json = { { "author", poll.author },
                  { "creationDateTime", poll.creationDateTime },
                  { "description", poll.description },
                  { "links", linksJson},
                  { "name", poll.name },
                  { "options", optionsJson },
                  { "totalVotes", poll.totalVotes } };

    std::stringbuf buf(json.dump());
    request->write(&buf);
    request->setStatus(HTTP_STATUS_SUCCESS);
}

void WebService::web_service_post_poll(fastcgi::Request* request) {
//    fastcgi::DataBuffer dataBuff = request->requestBody();
//    if (dataBuff.empty()) {
//        request->setStatus(HTTP_STATUS_BAD_REQUEST);
//        return;
//    }
//
//    std::string body;
//    dataBuff.toString(body);
//    auto json = json::parse(body);
//
//    if (!json["name"].is_string()
//        || !json["description"].is_string()
//        || !json["author"].is_string()
//        || !json["options"].is_array()
//        || json["options"].empty()) {
//        request->setStatus(HTTP_STATUS_BAD_REQUEST);
//        return;
//    }

    Poll poll(generateUUID());
//    poll.name = json["name"];
//    poll.description = json["description"];
//    poll.author = json["author"];
    poll.name = "name";
    poll.description = "description";
    poll.author = "author";

    std::vector<PollOption> options;
//    for (json::iterator it = json["options"].begin(); it != json["options"].end(); ++it) {
//        if ((*it).find("id") == (*it).end()
//            || (*it).find("name") == (*it).end()) {
//            request->setStatus(HTTP_STATUS_BAD_REQUEST);
//            return;
//        }
//        PollOption option;
//        option.id = it.value()["id"];
//        option.id = it.value()["name"];
//        options.push_back(option);
//    }
    PollOption option;
    option.id = 1;
    option.name = "name";
    options.push_back(option);

    poll.options = options;

    const char* message = nullptr;
    if (this->storageClient->poll_new(poll, &message) == QUERY_FAILURE) {
//        request->setStatus(HTTP_STATUS_INTERNAL_ERROR);
//        if (message) {
//            std::stringbuf buffer(message);
//            request->write(&buffer);
//        }
        return;
    }

//    request->setHeader("Location", "/polls/" + poll.getId());
//    request->setStatus(HTTP_STATUS_CREATED);
}

void WebService::web_service_delete_poll(fastcgi::Request* request, std::string const& id) {
    const char* message = nullptr;
    if (this->storageClient->poll_delete(id, &message) == QUERY_FAILURE) {
        request->setStatus(HTTP_STATUS_INTERNAL_ERROR);
        if (message) {
            std::stringbuf buffer(message);
            request->write(&buffer);
        }
        return;
    }

    request->setStatus(HTTP_STATUS_NO_CONTENT);
}

void WebService::web_service_get_votes(fastcgi::Request* request, std::string const& id) {
    const char *message = nullptr;
    std::vector<Vote> votes;

    if (this->storageClient->votes_get(id, votes, &message) == QUERY_FAILURE) {
        request->setStatus(HTTP_STATUS_INTERNAL_ERROR);
        if (message) {
            std::stringbuf buffer(message);
            request->write(&buffer);
        }
        return;
    }

    json dataJson;
    for (auto& vote : votes) {
        dataJson.push_back({{ "author", vote.author },
                            { "optionId" , vote.optionId },
                            { "links", { { "self", "/polls/" + id +  "/votes/" + vote.getId() } } }
                           });
    }

    json linksJson = { { "self", "/polls/" + id + "/votes" } };
    json json = { { "data", dataJson }, { "links", linksJson} };

    std::stringbuf buf(json.dump());
    request->write(&buf);
    request->setStatus(HTTP_STATUS_SUCCESS);
}

void WebService::web_service_get_vote(fastcgi::Request* request, std::string const& id, std::string const& pollid) {
    const char *message = nullptr;
    std::vector<Vote> votes;

    if (this->storageClient->vote_get(id, votes, &message) == QUERY_FAILURE) {
        request->setStatus(HTTP_STATUS_INTERNAL_ERROR);
        if (message) {
            std::stringbuf buffer(message);
            request->write(&buffer);
        }
        return;
    }

    if (votes.empty()) {
        request->setStatus(HTTP_STATUS_NOT_FOUND);
    }

    auto vote = votes.front();

    json linksJson = { { "self", "/polls/" + pollid + "/votes/" + id } };
    json json = { { "author", vote.author },
                  { "optionId", vote.optionId },
                  { "links", linksJson} };

    std::stringbuf buf(json.dump());
    request->write(&buf);
    request->setStatus(HTTP_STATUS_SUCCESS);
}

void WebService::web_service_post_vote(fastcgi::Request* request, std::string const& id) {
    fastcgi::DataBuffer dataBuff = request->requestBody();
    if (dataBuff.empty()) {
        request->setStatus(HTTP_STATUS_BAD_REQUEST);
        return;
    }

    std::string body;
    dataBuff.toString(body);
    auto json = json::parse(body);


    if (json.find("author") == json.end()
        || json.find("optionId") == json.end()) {
        request->setStatus(HTTP_STATUS_BAD_REQUEST);
        return;
    }

    Vote vote(generateUUID());
    vote.author = json["author"];
    vote.optionId = json["optionId"];

    const char* message = nullptr;
    if (this->storageClient->vote_new(id, vote, &message) == QUERY_FAILURE) {
        request->setStatus(HTTP_STATUS_INTERNAL_ERROR);
        if (message) {
            std::stringbuf buffer(message);
            request->write(&buffer);
        }
        return;
    }

    request->setHeader("Location", "/polls/" + id + "/votes/" + vote.getId());
    request->setStatus(HTTP_STATUS_CREATED);
}

void WebService::web_service_put_vote(fastcgi::Request* request, std::string const& id, std::string const& pollid) {
    fastcgi::DataBuffer dataBuff = request->requestBody();
    if (dataBuff.empty()) {
        request->setStatus(HTTP_STATUS_BAD_REQUEST);
        return;
    }

    std::string body;
    dataBuff.toString(body);
    auto json = json::parse(body);

    if (json.find("optionId") == json.end()
        || json.find("author") == json.end()) {
        request->setStatus(HTTP_STATUS_BAD_REQUEST);
        return;
    }


    Vote vote(id);
    vote.author = json["author"];
    vote.optionId = json["optionId"];

    const char* message = nullptr;
    if (this->storageClient->vote_update(pollid, vote, &message) == QUERY_FAILURE) {
        request->setStatus(HTTP_STATUS_INTERNAL_ERROR);
        if (message) {
            std::stringbuf buffer(message);
            request->write(&buffer);
        }
        return;
    }

    request->setStatus(HTTP_STATUS_SUCCESS);
}

void WebService::web_service_delete_vote(fastcgi::Request* request, std::string const& id, std::string const& pollid) {
    const char* message = nullptr;
    std::vector<Vote> votes;
    if (this->storageClient->vote_get(id, votes, &message) == QUERY_FAILURE) {
        request->setStatus(HTTP_STATUS_INTERNAL_ERROR);
        if (message) {
            std::stringbuf buffer(message);
            request->write(&buffer);
        }
        return;
    }

    if (votes.empty()) {
        request->setStatus(HTTP_STATUS_NOT_FOUND);
    }

    auto vote = votes.front();
    if (this->storageClient->vote_delete(pollid, vote, &message) == QUERY_FAILURE) {
        request->setStatus(HTTP_STATUS_INTERNAL_ERROR);
        if (message) {
            std::stringbuf buffer(message);
            request->write(&buffer);
        }
        return;
    }

    request->setStatus(HTTP_STATUS_NO_CONTENT);
}
