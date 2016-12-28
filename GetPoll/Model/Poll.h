//
// Created by ubuntu on 24.12.16.
//

#ifndef GETPOLL_POLL_H
#define GETPOLL_POLL_H

#include <string>
#include <vector>

#include "PollOption.h"

class Poll {
    std::string id;

public:
    std::string author, description, name, creationDateTime;
    std::vector<PollOption> options;
    int totalVotes;

    Poll(std::string id) : id(id) {};

    std::string const& getId() const;
};

#endif //GETPOLL_POLL_H
