//
// Created by ubuntu on 27.12.16.
//

#ifndef GETPOLL_WEBSERVICE_H
#define GETPOLL_WEBSERVICE_H

#include <fastcgi2/request.h>
#include <boost/thread/tss.hpp>
#include "../Storage/StorageClient.h"

class WebService {
public:
    StorageClient& storageClient;

    void web_service_get_polls(fastcgi::Request* request, std::string const* creationDateTime);

    void web_service_get_poll(fastcgi::Request* request, std::string const& id);
    void web_service_post_poll(fastcgi::Request* request);
    void web_service_delete_poll(fastcgi::Request* request, std::string const& id);

    void web_service_get_votes(fastcgi::Request* request, std::string const& id);

    void web_service_get_vote(fastcgi::Request* request, std::string const& id, std::string const& pollid);
    void web_service_post_vote(fastcgi::Request* request, std::string const& id);
    void web_service_put_vote(fastcgi::Request* request, std::string const& id, std::string const& pollid);
    void web_service_delete_vote(fastcgi::Request* request, std::string const& id, std::string const& pollid);

public:
    WebService() : storageClient(StorageClient::getInstance()) {};
    ~WebService() = default;

    void web_service_process_request(fastcgi::Request* request);

};


#endif //GETPOLL_WEBSERVICE_H
