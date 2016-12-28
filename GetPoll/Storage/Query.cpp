//
// Created by ubuntu on 26.12.16.
//

#include <clocale>
#include <string>
#include <sstream>
#include <functional>

#include "Query.h"
#include "../Model/Poll.h"
#include "../Model/Vote.h"


std::string select_polls_query(int limit, std::string const* creationDateTime) {
    std::stringstream sstream;
    sstream << "SELECT id, creationDateTime, name FROM getpoll.polls";
    if (creationDateTime != NULL) {
        sstream << " WHERE creationDateTime < ";
        sstream << *creationDateTime;
    }
    sstream << " LIMIT " << limit << ";";
    return sstream.str();
}

std::string select_poll_query(std::string const& pollid) {
    std::stringstream sstream;
    sstream << "SELECT creationDateTime, name, description, author, options FROM getpoll.polls WHERE id = ";
    sstream << pollid << ";";
    return sstream.str();
}

std::string select_poll_option_votes_query(std::string const& pollid) {
    std::stringstream sstream;
    sstream << "SELECT optionid, votes FROM getpoll.poll_option_votes WHERE pollid = ";
    sstream << pollid << ";";
    return sstream.str();
}

std::string select_poll_votes_query(std::string const& pollid) {
    std::stringstream sstream;
    sstream << "SELECT totalVotes FROM getpoll.poll_votes WHERE pollid = ";
    sstream << pollid << ";";
    return sstream.str();
}

std::string select_votes_query(std::string const& pollid) {
    std::stringstream sstream;
    sstream << "SELECT id, optionid, author FROM getpoll.votes WHERE pollid = ";
    sstream << pollid << ";";
    return sstream.str();
}

std::string select_vote_query(std::string const& id) {
    std::stringstream sstream;
    sstream << "SELECT optionid, author FROM getpoll.votes WHERE id = '";
    sstream << id << "';";
    return sstream.str();
}

std::string insert_poll_query(Poll const& poll) {

    std::stringstream sstream;
    sstream << "INSERT INTO getpoll.polls ( id, name, author, description, creationDateTime, options ) ";
    sstream << "VALUES( '";
    sstream << poll.getId() << "', '";
    sstream << poll.name << "', '";
    sstream << poll.author << "', '" << poll.description;
    sstream << "', now(), [ ";
    for (auto it = poll.options.begin(); it != poll.options.end(); ++it) {
        sstream << "{ id : " << it->id << ", name : '";
        sstream << it->name << "' }";
        if (it + 1 != poll.options.end()) {
            sstream << ", ";
        }
    }
    sstream << " ] ) IF NOT EXISTS;";
    return sstream.str();
}

std::string insert_vote_query(std::string const& pollid, Vote const& vote) {
    std::stringstream sstream;
    sstream << "INSERT INTO getpoll.votes ( id, pollid, optionid, author, hash_prefix ) ";
    sstream << "VALUES( '";
    sstream << vote.getId() << "', ";
    sstream << pollid << ", " << vote.optionId << ", '";

    std::hash<std::string> hash;
    sstream << vote.author << "', " <<  hash(pollid) % 2 << " ) ";
    sstream << "IF NOT EXISTS;";
    return sstream.str();
}

std::string insert_option_votes_query(std::string const& pollid, int optionid) {
    std::stringstream sstream;
    sstream << "INSERT INTO getpoll.poll_option_votes ( pollid, optionid) VALUES ( ";
    sstream << pollid << ", " << optionid << " ) IF NOT EXISTS;";
    return sstream.str();
}

std::string insert_poll_votes_query(std::string const& pollid) {
    std::stringstream sstream;
    sstream << "INSERT INTO getpoll.poll_votes ( pollid ) VALUES( ";
    sstream << pollid << " ) IF NOT EXISTS;";
    return  sstream.str();
}

std::string update_option_votes_query(std::string const& pollid, int optionid, int count_delta) {
    std::stringstream sstream;
    sstream << "UPDATE getpoll.poll_option_votes SET votes = votes + " << count_delta << " WHERE pollid = ";
    sstream << pollid << " AND optionid = " << optionid << " IF EXISTS;";
    return sstream.str();
}

std::string update_poll_votes_query(std::string const& pollid, int count_delta) {
    std::stringstream sstream;
    sstream << "UPDATE getpoll.poll_votes SET totalVotes = totalVotes + " << count_delta << " WHERE pollid = ";
    sstream << pollid << " IF EXISTS;";
    return  sstream.str();
}

std::string update_vote_query(Vote const& vote) {
    std::stringstream sstream;
    sstream << "UPDATE getpoll.votes SET optionid = " << vote.optionId << " WHERE id = '";
    sstream << vote.getId() << "' IF EXISTS;";
    return sstream.str();
}

std::string delete_poll_query(std::string const& pollid) {
    std::stringstream sstream;
    sstream << "DELETE id, name, author, description, creationDateTime, options FROM getpoll.polls ";
    sstream << "WHERE id = ";
    sstream << pollid << " IF EXISTS;";
    return sstream.str();
}

std::string delete_vote_query(std::string const& voteid) {
    std::stringstream sstream;
    sstream << "DELETE id, author, pollid, optionid FROM getpoll.votes WHERE id = '";
    sstream << voteid << "' IF EXISTS;";
    return sstream.str();
}

std::string delete_votes_query(std::string const& pollid) {
    std::stringstream sstream;
    sstream << "DELETE id, author, pollid, optionid FROM getpoll.votes WHERE pollid = ";
    sstream << pollid << " IF EXISTS;";
    return sstream.str();
}

std::string delete_poll_votes_query(std::string const& pollid) {
    std::stringstream sstream;
    sstream << "DELETE pollid, totalVotes FROM getpoll.poll_votes WHERE pollid = ";
    sstream << pollid << " IF EXISTS;";
    return sstream.str();
}

std::string delete_option_votes_query(std::string const& pollid) {
    std::stringstream sstream;
    sstream << "DELETE pollid, optionid, votes FROM getpoll.poll_option_votes WHERE pollid = ";
    sstream << pollid << " IF EXISTS;";
    return sstream.str();
}
