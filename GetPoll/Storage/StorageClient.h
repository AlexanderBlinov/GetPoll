//
// Created by ubuntu on 25.12.16.
//

#ifndef GETPOLL_STORAGECLIENT_H
#define GETPOLL_STORAGECLIENT_H

#include <string>
#include <vector>

#include "../Model/Poll.h"
#include "../Model/PollOption.h"
#include "../Model/Vote.h"
#include "../CassandraDriver/cassandra.h"

extern int const QUERY_SUCCESS;
extern int const QUERY_FAILURE;

class StorageClient {
 private:
     CassCluster* cluster;
     CassSession* session;

     CassCluster* getCluster();
     CassSession* getSession();

     CassFuture* connect(const char** message);

     int perform_query(const char *query, const CassResult** result, const char **message);
     int perform_batch_query(std::vector<std::string>& queries, const char**message);

 public:
     const char* hosts;

     StorageClient(const char* hosts);
     ~StorageClient();

     int polls_get(std::string& creationDateTime, std::vector<Poll>& result, const char** message);
     int poll_get(std::string& id, std::vector<Poll>& result, const char** message);

     int votes_get(std::string& pollid, std::vector<Vote>& result, const char** message);
     int vote_get(std::string& id, std::vector<Vote>& result, const char** message);

     int poll_new(Poll const& poll, const char** message);
     int vote_new(std::string& pollid, Vote const& vote, const char** message);

     int vote_update(std::string& pollid, Vote const& vote, const char** message);

     int poll_delete(std::string& id, const char** message);
     int vote_delete(std::string& pollid, Vote const& vote, const char** message);
 };



#endif //GETPOLL_STORAGECLIENT_H