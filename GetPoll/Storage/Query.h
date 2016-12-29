//
// Created by ubuntu on 26.12.16.
//

#ifndef GETPOLL_QUERY_H
#define GETPOLL_QUERY_H

#include "../CassandraDriver/cassandra.h"

class Poll;
class Vote;

CassStatement* select_polls_query(int limit, std::string const* creationDateTime);

CassStatement* select_poll_query(std::string const& pollid);

CassStatement* select_poll_votes_query(std::string const& pollid);

CassStatement* select_poll_option_votes_query(std::string const& pollid);

CassStatement* select_votes_query(std::string const& pollid);

CassStatement* select_vote_query(std::string const& id);

CassStatement* insert_poll_query(Poll const& poll);

CassStatement* insert_vote_query(std::string const& pollid, Vote const& vote);

CassStatement* update_option_votes_query(std::string const& pollid, int optionid, long long count_delta);

CassStatement* update_poll_votes_query(std::string const& pollid, long long count_delta);

CassStatement* update_vote_query(Vote const& vote);

CassStatement* delete_poll_query(std::string const& pollid);

CassStatement* delete_vote_query(std::string const& voteid);

CassStatement* delete_votes_query(std::string const& pollid);

#endif //GETPOLL_QUERY_H
