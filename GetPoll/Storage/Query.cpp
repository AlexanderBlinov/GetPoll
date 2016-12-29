//
// Created by ubuntu on 26.12.16.
//

#include <clocale>
#include <string>
#include <sstream>
#include <functional>

#include "../Model/Poll.h"
#include "../Model/Vote.h"

#include "Query.h"


CassUuid getUUID(std::string const& uuidStr) {
    CassUuid uuid;
    cass_uuid_from_string(uuidStr.c_str(), &uuid);
    return uuid;
}

CassStatement* select_polls_query(int limit, std::string const* creationDateTime) {
    size_t params = 1;

    std::stringstream sstream;
    sstream << "SELECT id, creationDateTime, name FROM getpoll.polls";
    if (creationDateTime != NULL) {
        sstream << " WHERE creationDateTime < ? ";
        ++params;
    }
    sstream << " LIMIT ?;";

    CassStatement* statement = cass_statement_new(sstream.str().c_str(), params);
    switch (params) {
        case 1:
            cass_statement_bind_int32(statement, 0, limit);
            break;
        case 2:
            cass_statement_bind_string(statement, 0, creationDateTime->c_str());
            cass_statement_bind_int32(statement, 1, limit);
            break;
        default:
            break;
    }

    return statement;
}

CassStatement* select_poll_query(std::string const& pollid) {
    const char* query = "SELECT creationDateTime, name, description, author, options FROM getpoll.polls WHERE id = ?;";
    CassStatement* statement = cass_statement_new(query, 1);
    cass_statement_bind_uuid(statement, 0, getUUID(pollid));

    return statement;
}

CassStatement* select_poll_option_votes_query(std::string const& pollid) {
    const char* query = "SELECT optionid, votes FROM getpoll.poll_option_votes WHERE pollid = ?;";
    CassStatement* statement = cass_statement_new(query, 1);
    cass_statement_bind_uuid(statement, 0, getUUID(pollid));

    return statement;
}

CassStatement* select_poll_votes_query(std::string const& pollid) {
    const char* query = "SELECT totalVotes FROM getpoll.poll_votes WHERE pollid = ?;";
    CassStatement* statement = cass_statement_new(query, 1);
    cass_statement_bind_uuid(statement, 0, getUUID(pollid));

    return statement;
}

CassStatement* select_votes_query(std::string const& pollid) {
    const char* query = "SELECT id, optionid, author FROM getpoll.votes WHERE pollid = ?;";
    CassStatement* statement = cass_statement_new(query, 1);
    cass_statement_bind_uuid(statement, 0, getUUID(pollid));

    return statement;
}

CassStatement* select_vote_query(std::string const& id) {
    const char* query = "SELECT optionid, author FROM getpoll.votes WHERE id = ?;";
    CassStatement* statement = cass_statement_new(query, 1);
    cass_statement_bind_uuid(statement, 0, getUUID(id));

    return statement;
}

CassStatement* insert_poll_query(Poll const& poll) {
    std::stringstream sstream;
    sstream << "INSERT INTO getpoll.polls ( id, name, author, description, creationDateTime, options ) ";
    sstream << "VALUES( ?, ?, ?, ?, toUnixTimestamp(now()), [ ";
    for (auto it = poll.options.begin(); it != poll.options.end(); ++it) {
        sstream << "{ id : " << it->id << ", name : '";
        sstream << it->name << "' }";
        if (it + 1 != poll.options.end()) {
            sstream << ", ";
        }
    }
    sstream << " ] ) IF NOT EXISTS;";

    CassStatement* statement = cass_statement_new(sstream.str().c_str(), 4);
    cass_statement_bind_uuid(statement, 0, getUUID(poll.getId()));
    cass_statement_bind_string(statement, 1, poll.name.c_str());
    cass_statement_bind_string(statement, 2, poll.author.c_str());
    cass_statement_bind_string(statement, 3, poll.description.c_str());

    return statement;
}

CassStatement* insert_vote_query(std::string const& pollid, Vote const& vote) {
    const char* query = "INSERT INTO getpoll.votes ( id, pollid, optionid, author, hash_prefix ) VALUES( ?, ?, ?, ?, ? ) IF NOT EXISTS;";

    CassStatement* statement = cass_statement_new(query, 5);
    cass_statement_bind_uuid(statement, 0, getUUID(vote.getId()));
    cass_statement_bind_uuid(statement, 1, getUUID(pollid));
    cass_statement_bind_int32(statement, 2, vote.optionId);
    cass_statement_bind_string(statement, 3, vote.author.c_str());

    std::hash<std::string> hash;
    cass_statement_bind_int32(statement, 4, hash(pollid) % 2);

    return statement;
}

CassStatement* update_option_votes_query(std::string const& pollid, int optionid, long long count_delta) {
    CassStatement* statement = cass_statement_new("UPDATE getpoll.poll_option_votes SET votes = votes + ? WHERE pollid = ? AND optionid = ?;", 3);
    cass_statement_bind_int64(statement, 0, count_delta);
    cass_statement_bind_uuid(statement, 1, getUUID(pollid));
    cass_statement_bind_int32(statement, 2, optionid);

    return statement;
}

CassStatement* update_poll_votes_query(std::string const& pollid, long long count_delta) {
    CassStatement* statement = cass_statement_new("UPDATE getpoll.poll_votes SET totalVotes = totalVotes + ? WHERE pollid = ?;", 2);
    cass_statement_bind_int64(statement, 0, count_delta);
    cass_statement_bind_uuid(statement, 1, getUUID(pollid));

    return statement;
}

CassStatement* update_vote_query(Vote const& vote) {
    CassStatement* statement = cass_statement_new("UPDATE getpoll.votes SET optionid = ?  WHERE id = ?;", 2);
    cass_statement_bind_int32(statement, 0, vote.optionId);
    cass_statement_bind_uuid(statement, 1, getUUID(vote.getId()));

    return statement;
}

CassStatement* delete_poll_query(std::string const& pollid) {
    CassStatement* statement = cass_statement_new("DELETE FROM getpoll.polls WHERE id = ?;", 1);
    cass_statement_bind_uuid(statement, 0, getUUID(pollid));

    return statement;
}

CassStatement* delete_vote_query(std::string const& voteid) {
    CassStatement* statement = cass_statement_new("DELETE FROM getpoll.votes WHERE id = ? AND hash_prefix IN (0, 1);", 1);
    cass_statement_bind_uuid(statement, 0, getUUID(voteid));

    return statement;
}

CassStatement* delete_votes_query(std::string const& pollid) {
    CassStatement* statement = cass_statement_new("DELETE FROM getpoll.votes WHERE pollid = ? AND hash_prefix IN (0, 1);", 1);
    cass_statement_bind_uuid(statement, 0, getUUID(pollid));

    return statement;
}
