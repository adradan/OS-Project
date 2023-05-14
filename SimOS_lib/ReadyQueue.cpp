//
// Created by h3yb4ws on 5/13/2023.
//

#include "ReadyQueue.h"

int ReadyQueue::moveUpQueue() {
    if (readyQueue.empty()) {
        return 0;
    }
    int newRunner = readyQueue.front();
    readyQueue.erase(readyQueue.begin());
    return newRunner;
}

std::vector<int> ReadyQueue::getQueue() {
    return readyQueue;
}
