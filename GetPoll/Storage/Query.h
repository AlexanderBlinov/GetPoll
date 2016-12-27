//
// Created by ubuntu on 26.12.16.
//

#ifndef GETPOLL_QUERY_H
#define GETPOLL_QUERY_H

#include "../Model/Poll.h"
#include "../Model/Vote.h"

const char* select_polls_query(int limit, const char* creationDateTime);

const char* select_poll_query(const char* pollid);

const char* select_poll_votes_query(const char* pollid);

const char* select_poll_option_votes_query(const char* pollid);

const char* select_votes_query(const char* pollid);

const char* select_vote_query(const char* id);

const char* insert_poll_query(Poll const& poll);

const char* insert_vote_query(const char* pollid, Vote const& vote);

const char* insert_option_votes_query(const char* pollid, int optionid);

const char* insert_poll_votes_query(const char* pollid);

const char* update_option_votes_query(const char* pollid, int optionid, int count_delta);

const char* update_poll_votes_query(const char* pollid, int count_delta);

const char* update_vote_query(Vote const& vote);

const char* delete_poll_query(const char* pollid);

const char* delete_vote_query(const char* voteid);

const char* delete_votes_query(const char* pollid);

const char* delete_poll_votes_query(const char* pollid);

const char* delete_option_votes_query(const char* pollid);


#endif //GETPOLL_QUERY_H
