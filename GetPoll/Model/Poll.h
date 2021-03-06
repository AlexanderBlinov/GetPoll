//
// Created by ubuntu on 24.12.16.
//

#ifndef GETPOLL_POLL_H
#define GETPOLL_POLL_H

#include <string>
#include <vector>

#include "PollOption.h"

class Poll {
private:
    std::string id;

public:
    std::string author, description, name, creationDateTime;
    std::vector<PollOption> options;
    long long totalVotes;

    Poll(std::string id) : id(id), totalVotes(0) {};

    std::string const& getId() const;

    bool operator>(Poll const& poll) const;
};

#endif //GETPOLL_POLL_H
