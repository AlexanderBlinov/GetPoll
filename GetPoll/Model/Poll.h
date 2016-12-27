//
// Created by ubuntu on 24.12.16.
//

#ifndef GETPOLL_POLL_H
#define GETPOLL_POLL_H

#include <string>
#include <vector>

#include "PollOption.h"

class Poll {
public:
    std::string id, author, description, name, creationDateTime;
    std::vector<std::string> links;
    std::vector<PollOption> options;
    int totalVotes;
};

#endif //GETPOLL_POLL_H
