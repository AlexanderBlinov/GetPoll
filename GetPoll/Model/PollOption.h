//
// Created by ubuntu on 24.12.16.
//

#ifndef GETPOLL_POLLOPTION_H
#define GETPOLL_POLLOPTION_H

#include <string>

 class PollOption {
 public:
     std::string name;
     int id;
     long long votes;

     PollOption() : votes(0) {};
 };

#endif //GETPOLL_POLLOPTION_H
