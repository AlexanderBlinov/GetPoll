//
// Created by ubuntu on 24.12.16.
//

#include "Poll.h"


std::string const &Poll::getId() const {
    return id;
}

bool Poll::operator>(Poll const &poll) const {
    if (creationDateTime.compare(poll.creationDateTime) >= 0) {
        return true;
    }
    return false;
}
