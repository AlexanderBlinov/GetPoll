//
// Created by ubuntu on 26.12.16.
//

#include <clocale>
#include <string>
#include <sstream>
#include <functional>
#include <chrono>
#include <string.h>
#include <time.h>

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
        sstream << " WHERE creationDateTime < '";
        sstream << creationDateTime->c_str() << "'";
    }
    sstream << " LIMIT ? ALLOW FILTERING;";

    CassStatement* statement = cass_statement_new(sstream.str().c_str(), params);
    cass_statement_bind_int32(statement, 0, limit);

    cass_statement_set_consistency(statement, CASS_CONSISTENCY_ONE);

    return statement;
}

CassStatement* select_poll_query(std::string const& pollid) {
    const char* query = "SELECT creationDateTime, name, description, author, options FROM getpoll.polls WHERE id = ?;";
    CassStatement* statement = cass_statement_new(query, 1);
    cass_statement_bind_uuid(statement, 0, getUUID(pollid));

    cass_statement_set_consistency(statement, CASS_CONSISTENCY_ONE);

    return statement;
}

CassStatement* select_poll_option_votes_query(std::string const& pollid) {
    const char* query = "SELECT optionid, votes FROM getpoll.poll_option_votes WHERE pollid = ?;";
    CassStatement* statement = cass_statement_new(query, 1);
    cass_statement_bind_uuid(statement, 0, getUUID(pollid));

    cass_statement_set_consistency(statement, CASS_CONSISTENCY_ONE);

    return statement;
}

CassStatement* select_poll_votes_query(std::string const& pollid) {
    const char* query = "SELECT totalVotes FROM getpoll.poll_votes WHERE pollid = ?;";
    CassStatement* statement = cass_statement_new(query, 1);
    cass_statement_bind_uuid(statement, 0, getUUID(pollid));

    cass_statement_set_consistency(statement, CASS_CONSISTENCY_ONE);

    return statement;
}

CassStatement* select_votes_query(std::string const& pollid) {
    const char* query = "SELECT id, optionid, author FROM getpoll.votes WHERE pollid = ? AND hash_prefix IN (1, 0);";
    CassStatement* statement = cass_statement_new(query, 1);
    cass_statement_bind_uuid(statement, 0, getUUID(pollid));

    cass_statement_set_consistency(statement, CASS_CONSISTENCY_ONE);

    return statement;
}

CassStatement* select_vote_query(std::string const& id, std::string const& pollid) {
    const char* query = "SELECT optionid, author FROM getpoll.votes WHERE id = ? AND pollid = ? AND hash_prefix IN (0, 1);";
    CassStatement* statement = cass_statement_new(query, 2);
    cass_statement_bind_uuid(statement, 0, getUUID(id));
    cass_statement_bind_uuid(statement, 1, getUUID(pollid));

    cass_statement_set_consistency(statement, CASS_CONSISTENCY_ONE);

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

    cass_statement_set_consistency(statement, CASS_CONSISTENCY_QUORUM);

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

    cass_statement_set_consistency(statement, CASS_CONSISTENCY_QUORUM);

    return statement;
}

CassStatement* update_option_votes_query(std::string const& pollid, int optionid, long long count_delta) {
    CassStatement* statement = cass_statement_new("UPDATE getpoll.poll_option_votes SET votes = votes + ? WHERE pollid = ? AND optionid = ?;", 3);
    cass_statement_bind_int64(statement, 0, count_delta);
    cass_statement_bind_uuid(statement, 1, getUUID(pollid));
    cass_statement_bind_int32(statement, 2, optionid);

    cass_statement_set_consistency(statement, CASS_CONSISTENCY_QUORUM);

    return statement;
}

CassStatement* update_poll_votes_query(std::string const& pollid, long long count_delta) {
    CassStatement* statement = cass_statement_new("UPDATE getpoll.poll_votes SET totalVotes = totalVotes + ? WHERE pollid = ?;", 2);
    cass_statement_bind_int64(statement, 0, count_delta);
    cass_statement_bind_uuid(statement, 1, getUUID(pollid));

    cass_statement_set_consistency(statement, CASS_CONSISTENCY_QUORUM);

    return statement;
}

CassStatement* update_vote_query(Vote const& vote, std::string const& pollid) {
    CassStatement* statement = cass_statement_new("UPDATE getpoll.votes SET optionid = ?  WHERE id = ? AND pollid = ? AND hash_prefix IN (0, 1);", 3);
    cass_statement_bind_int32(statement, 0, vote.optionId);
    cass_statement_bind_uuid(statement, 1, getUUID(vote.getId()));
    cass_statement_bind_uuid(statement, 2, getUUID(pollid));

    cass_statement_set_consistency(statement, CASS_CONSISTENCY_QUORUM);

    return statement;
}

CassStatement* delete_poll_query(std::string const& pollid) {
    CassStatement* statement = cass_statement_new("DELETE FROM getpoll.polls WHERE id = ?;", 1);
    cass_statement_bind_uuid(statement, 0, getUUID(pollid));

    cass_statement_set_consistency(statement, CASS_CONSISTENCY_QUORUM);

    return statement;
}

CassStatement* delete_vote_query(std::string const& voteid, std::string const& pollid) {
    CassStatement* statement = cass_statement_new("DELETE FROM getpoll.votes WHERE id = ? AND pollid = ? AND hash_prefix IN (0, 1);", 2);
    cass_statement_bind_uuid(statement, 0, getUUID(voteid));
    cass_statement_bind_uuid(statement, 1, getUUID(pollid));

    cass_statement_set_consistency(statement, CASS_CONSISTENCY_QUORUM);

    return statement;
}

CassStatement* delete_votes_query(std::string const& pollid) {
    CassStatement* statement = cass_statement_new("DELETE FROM getpoll.votes WHERE pollid = ? AND hash_prefix IN (0, 1);", 1);
    cass_statement_bind_uuid(statement, 0, getUUID(pollid));

    cass_statement_set_consistency(statement, CASS_CONSISTENCY_QUORUM);

    return statement;
}
