//
// Created by ubuntu on 27.12.16.
//

#include <iostream>

#include "Network/WebService.h"
#include "Storage/StorageClient.h"

int main (int argc, char* argv[]) {
//    WebService *serv = new WebService();
//    serv->web_service_get_poll(nullptr, "9ab564a0-cd22-11e6-904d-79b4b55c8a00");
//    serv->web_service_post_poll(nullptr);
//    serv->web_service_delete_poll(nullptr, "79f6ac40-cd47-11e6-befd-79b4b55c8a00");

    StorageClient client("127.0.0.1");
    const char* message = nullptr;
    std::vector<Poll> polls;
    client.polls_get(NULL, polls, &message);
    return 0;
}

