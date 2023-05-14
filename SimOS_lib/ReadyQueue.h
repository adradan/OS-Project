//
// Created by h3yb4ws on 5/13/2023.
//

#ifndef SIMOS_READYQUEUE_H
#define SIMOS_READYQUEUE_H


#include <vector>

class ReadyQueue {
private:
    std::vector<int> readyQueue;
public:
    int moveUpQueue();
    std::vector<int> getQueue();

};


#endif //SIMOS_READYQUEUE_H
