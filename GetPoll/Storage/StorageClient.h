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
    CassUuidGen* uuidGen;

    const char* hosts;
    bool connected;

    void connect(const char** message);

    int perform_query(CassStatement* statement, const CassResult** result, const char **message);int perform_batch_query(CassBatchType type, std::vector<CassStatement*>& statements, const char**message);

 public:


    StorageClient(const char* hosts);
    ~StorageClient();

    std::string generateUUID();

    int polls_get(std::string const* creationDateTime, std::vector<Poll>& result, const char** message);
    int poll_get(std::string const& id, std::vector<Poll>& result, const char** message);

    int votes_get(std::string const& pollid, std::vector<Vote>& result, const char** message);
    int vote_get(std::string const& id, std::string const& pollid, std::vector<Vote>& result, const char** message);

    int poll_new(Poll const& poll, const char** message);
    int vote_new(std::string const& pollid, Vote const& vote, const char** message);

    int vote_update(std::string const& pollid, Vote const& vote, const char** message);

    int poll_delete(std::string const& id, const char** message);
    int vote_delete(std::string const& pollid, Vote const& vote, const char** message);
 };



#endif //GETPOLL_STORAGECLIENT_H
