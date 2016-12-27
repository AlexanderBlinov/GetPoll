//
// Created by ubuntu on 26.12.16.
//

#include <clocale>
#include <string>
#include <sstream>
#include <functional>

#include "Query.h"


std::string select_polls_query(int limit, std::string const* creationDateTime) {
    std::string string;
    std::stringstream sstream(string);
    sstream << "SELECT id, creationDateTime, name FROM getpoll.polls";
    if (creationDateTime != NULL) {
        sstream << " WHERE creationDateTime < ";
        sstream << *creationDateTime;
    }
    sstream << " LIMIT " << limit << ";";
    return string;
}

std::string select_poll_query(std::string const& pollid) {
    std::string string;
    std::stringstream sstream(string);
    sstream << "SELECT creationDateTime, name, description, author, options FROM getpoll.polls WHERE id = ";
    sstream << pollid << ";";
    return string;
}

std::string select_poll_option_votes_query(std::string const& pollid) {
    std::string string;
    std::stringstream sstream(string);
    sstream << "SELECT optionid, votes FROM getpoll.poll_option_votes WHERE pollid = ";
    sstream << pollid << ";";
    return string;
}

std::string select_poll_votes_query(std::string const& pollid) {
    std::string string;
    std::stringstream sstream(string);
    sstream << "SELECT totalVotes FROM getpoll.poll_votes WHERE pollid = ";
    sstream << pollid << ";";
    return string;
}

std::string select_votes_query(std::string const& pollid) {
    std::string string;
    std::stringstream sstream(string);
    sstream << "SELECT id, optionid, author FROM getpoll.votes WHERE pollid = '";
    sstream << pollid << "';";
    return string;
}

std::string select_vote_query(std::string const& id) {
    std::string string;
    std::stringstream sstream(string);
    sstream << "SELECT optionid, author FROM getpoll.votes WHERE id = '";
    sstream << id << "';";
    return string;
}

std::string insert_poll_query(Poll const& poll) {
    std::string string;
    std::stringstream sstream(string);
    sstream << "INSERT INTO getpoll.polls ( id, name, author, description, createDateTime, options ) ";
    sstream << "VALUES( now(), '";
    sstream << poll.name << "', '";
    sstream << poll.author << "', '" << poll.description;
    sstream << "', now(), { ";
    for (std::vector<PollOption>::const_iterator it = poll.options.begin(); it != poll.options.end(); ++it) {
        sstream << "{ id : '" << it->id << "', name : '";
        sstream << it->name << "' }";
        if (it + 1 != poll.options.end()) {
            sstream << ", ";
        }
    }
    sstream << " } ) IF NOT EXISTS;";
    return string;
}

std::string insert_vote_query(std::string const& pollid, Vote const& vote) {
    std::string string;
    std::stringstream sstream(string);
    sstream << "INSERT INTO getpoll.votes ( id, pollid, optionid, author, hash_prefix ) ";
    sstream << "VALUES( now(), '";
    sstream << pollid << "', " << vote.optionId << ", '";

    std::hash<std::string> hash;
    sstream << vote.author << "', " <<  hash(pollid) % 2 << " ) ";
    sstream << "IF NOT EXISTS;";
    return string;
}

std::string insert_option_votes_query(std::string const& pollid, int optionid) {
    std::string string;
    std::stringstream sstream(string);
    sstream << "INSERT INTO getpoll.poll_option_votes ( pollid, optionid) VALUES ( '";
    sstream << pollid << "', " << optionid << " ) IF NOT EXISTS;";
    return string;
}

std::string insert_poll_votes_query(std::string const& pollid) {
    std::string string;
    std::stringstream sstream(string);
    sstream << "INSERT INTO getpoll.poll_votes ( pollid ) VALUES( '";
    sstream << pollid << "' ) IF NOT EXISTS;";
    return  string;
}

std::string update_option_votes_query(std::string const& pollid, int optionid, int count_delta) {
    std::string string;
    std::stringstream sstream(string);
    sstream << "UPDATE getpoll.poll_option_votes SET votes = votes + " << count_delta << " WHERE pollid = '";
    sstream << pollid << "' AND optionid = " << optionid << " IF EXISTS;";
    return string;
}

std::string update_poll_votes_query(std::string const& pollid, int count_delta) {
    std::string string;
    std::stringstream sstream(string);
    sstream << "UPDATE getpoll.poll_votes SET totalVotes = totalVotes + " << count_delta << " WHERE pollid = '";
    sstream << pollid << "' IF EXISTS;";
    return  string;
}

std::string update_vote_query(Vote const& vote) {
    std::string string;
    std::stringstream sstream(string);
    sstream << "UPDATE getpoll.votes SET optionid = " << vote.optionId << " WHERE id = '";
    sstream << vote.getId() << "' IF EXISTS;";
    return string;
}

std::string delete_poll_query(std::string const& pollid) {
    std::string string;
    std::stringstream sstream(string);
    sstream << "DELETE id, name, author, description, createDateTime, options FROM getpoll.polls ";
    sstream << "WHERE id = '";
    sstream << pollid << "' IF EXISTS;";
    return string;
}

std::string delete_vote_query(std::string const& voteid) {
    std::string string;
    std::stringstream sstream(string);
    sstream << "DELETE id, author, pollid, optionid FROM getpoll.votes WHERE id = '";
    sstream << voteid << "' IF EXISTS;";
    return string;
}

std::string delete_votes_query(std::string const& pollid) {
    std::string string;
    std::stringstream sstream(string);
    sstream << "DELETE id, author, pollid, optionid FROM getpoll.votes WHERE pollid = '";
    sstream << pollid << "' IF EXISTS;";
    return string;
}

std::string delete_poll_votes_query(std::string const& pollid) {
    std::string string;
    std::stringstream sstream(string);
    sstream << "DELETE pollid, totalVotes FROM getpoll.poll_votes WHERE pollid = '";
    sstream << pollid << "' IF EXISTS;";
    return string;
}

std::string delete_option_votes_query(std::string const& pollid) {
    std::string string;
    std::stringstream sstream(string);
    sstream << "DELETE pollid, optionid, votes FROM getpoll.poll_option_votes WHERE pollid = '";
    sstream << pollid << "' IF EXISTS;";
    return string;
}
