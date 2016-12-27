//
// Created by ubuntu on 24.12.16.
//

#ifndef GETPOLL_VOTE_H
#define GETPOLL_VOTE_H

#include <string>

class Vote {
    std::string id;

public:
    std::string author;
    int optionId;

    Vote(std::string id) : id(id) {};

    std::string const& getId() const;
};

#endif //GETPOLL_VOTE_H
