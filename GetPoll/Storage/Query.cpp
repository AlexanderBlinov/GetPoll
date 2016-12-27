//
// Created by ubuntu on 26.12.16.
//

#include <clocale>
#include <string>
#include <sstream>

#include "Query.h"


const char* select_polls_query(int limit, const char* creationDateTime) {
    std::string string;
    std::stringstream sstream(string);
    sstream << "SELECT id, creationDateTime, name FROM getpoll.polls";
    if (creationDateTime != NULL) {
        sstream << " WHERE creationDateTime < " << creationDateTime;
    }
    sstream << " LIMIT " << limit << ";";
    return string.c_str();
}

const char* select_poll_query(const char *pollid) {
    std::string string;
    std::stringstream sstream(string);
    sstream << "SELECT creationDateTime, name, description, author, options FROM getpoll.polls WHERE id = ";
    sstream << pollid << ";";
    return string.c_str();
}

const char* select_poll_option_votes_query(const char *pollid) {
    std::string string;
    std::stringstream sstream(string);
    sstream << "SELECT optionid, votes FROM getpoll.poll_option_votes WHERE pollid = " << pollid << ";";
    return string.c_str();
}

const char* select_poll_votes_query(const char *pollid) {
    std::string string;
    std::stringstream sstream(string);
    sstream << "SELECT totalVotes FROM getpoll.poll_votes WHERE pollid = " << pollid << ";";
    return string.c_str();
}

const char* select_votes_query(const char* pollid) {
    std::string string;
    std::stringstream sstream(string);
    sstream << "SELECT id, optionid, author, creationDateTime FROM getpoll.votes WHERE pollid = '";
    sstream << pollid << "';";
    return string.c_str();
}

const char* select_vote_query(const char* id) {
    std::string string;
    std::stringstream sstream(string);
    sstream << "SELECT optionid, author, creationDateTime FROM getpoll.votes WHERE id = '";
    sstream << id << "';";
    return string.c_str();
}

const char* insert_poll_query(Poll const& poll) {
    std::string string;
    std::stringstream sstream(string);
    sstream << "INSERT INTO getpoll.polls ( id, name, author, description, createDateTime, options ) ";
    sstream << "VALUES( now(), '" << poll.name.c_str() << "', '";
    sstream << poll.author.c_str() << "', '" << poll.description.c_str();
    sstream << "', now(), { ";
    for (std::vector<PollOption>::const_iterator it = poll.options.begin(); it != poll.options.end(); ++it) {
        sstream << "{ id : '" << it->id << "', name : '" << it->name.c_str() << "' }";
        if (it + 1 != poll.options.end()) {
            sstream << ", ";
        }
    }
    sstream << " } ) IF NOT EXISTS;";
    return string.c_str();
}

const char* insert_vote_query(const char* pollid, Vote const& vote) {
    std::string string;
    std::stringstream sstream(string);
    sstream << "INSERT INTO getpoll.votes ( id , pollid, optionid, author, creationDateTime ) ";
    sstream << "VALUES( now(), '" << pollid << "', " << vote.optionId << ", '" << vote.author.c_str() << "', now() ) ";
    sstream << "IF NOT EXISTS;";
    return string.c_str();
}

const char* insert_option_votes_query(const char* pollid, int optionid) {
    std::string string;
    std::stringstream sstream(string);
    sstream << "INSERT INTO getpoll.poll_option_votes ( pollid, optionid) VALUES ( '";
    sstream << pollid << "', " << optionid << " ) IF NOT EXISTS;";
    return string.c_str();
}

const char* insert_poll_votes_query(const char* pollid) {
    std::string string;
    std::stringstream sstream(string);
    sstream << "INSERT INTO getpoll.poll_votes ( pollid ) VALUES( '" << pollid << "' ) IF NOT EXISTS;";
    return  string.c_str();
}

const char* update_option_votes_query(const char* pollid, int optionid, int count_delta) {
    std::string string;
    std::stringstream sstream(string);
    sstream << "UPDATE getpoll.poll_option_votes SET votes = votes + " << count_delta << " WHERE pollid = '";
    sstream << pollid << "' AND optionid = " << optionid << " IF EXISTS;";
    return string.c_str();
}

const char* update_poll_votes_query(const char* pollid, int count_delta) {
    std::string string;
    std::stringstream sstream(string);
    sstream << "UPDATE getpoll.poll_votes SET totalVotes = totalVotes + " << count_delta << " WHERE pollid = '";
    sstream << pollid << "' IF EXISTS;";
    return  string.c_str();
}

const char* update_vote_query(Vote const& vote) {
    std::string string;
    std::stringstream sstream(string);
    sstream << "UPDATE getpoll.votes SET optionid = " << vote.optionId << " WHERE id = '";
    sstream << vote.id.c_str() << "' IF EXISTS;";
    return string.c_str();
}

const char* delete_poll_query(const char* pollid) {
    std::string string;
    std::stringstream sstream(string);
    sstream << "DELETE id, name, author, description, createDateTime, options FROM getpoll.polls ";
    sstream << "WHERE id = '" << pollid << "' IF EXISTS;";
    return string.c_str();
}

const char* delete_vote_query(const char* voteid) {
    std::string string;
    std::stringstream sstream(string);
    sstream << "DELETE id, author, pollid, createDateTime, optionid FROM getpoll.votes ";
    sstream << "WHERE id = '" << voteid << "' IF EXISTS;";
    return string.c_str();
}

const char* delete_votes_query(const char* pollid) {
    std::string string;
    std::stringstream sstream(string);
    sstream << "DELETE id, author, pollid, createDateTime, optionid FROM getpoll.votes ";
    sstream << "WHERE pollid = '" << pollid << "' IF EXISTS;";
    return string.c_str();
}

const char* delete_poll_votes_query(const char* pollid) {
    std::string string;
    std::stringstream sstream(string);
    sstream << "DELETE pollid, totalVotes FROM getpoll.poll_votes ";
    sstream << "WHERE pollid = '" << pollid << "' IF EXISTS;";
    return string.c_str();
}

const char* delete_option_votes_query(const char* pollid) {
    std::string string;
    std::stringstream sstream(string);
    sstream << "DELETE pollid, optionid, votes FROM getpoll.poll_option_votes ";
    sstream << "WHERE pollid = '" << pollid << "' IF EXISTS;";
    return string.c_str();
}
