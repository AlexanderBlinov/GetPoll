//
// Created by ubuntu on 26.12.16.
//

#ifndef GETPOLL_QUERY_H
#define GETPOLL_QUERY_H

class Poll;
class Vote;

std::string select_polls_query(int limit, std::string const* creationDateTime);

std::string select_poll_query(std::string const& pollid);

std::string select_poll_votes_query(std::string const& pollid);

std::string select_poll_option_votes_query(std::string const& pollid);

std::string select_votes_query(std::string const& pollid);

std::string select_vote_query(std::string const& id);

std::string insert_poll_query(Poll const& poll);

std::string insert_vote_query(std::string const& pollid, Vote const& vote);

std::string insert_option_votes_query(std::string const& pollid, int optionid);

std::string insert_poll_votes_query(std::string const& pollid);

std::string update_option_votes_query(std::string const& pollid, int optionid, int count_delta);

std::string update_poll_votes_query(std::string const& pollid, int count_delta);

std::string update_vote_query(Vote const& vote);

std::string delete_poll_query(std::string const& pollid);

std::string delete_vote_query(std::string const& voteid);

std::string delete_votes_query(std::string const& pollid);

std::string delete_poll_votes_query(std::string const& pollid);

std::string delete_option_votes_query(std::string const& pollid);

#endif //GETPOLL_QUERY_H
