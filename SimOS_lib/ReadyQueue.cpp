//
// Created by h3yb4ws on 5/13/2023.
//

#include "ReadyQueue.h"

void ReadyQueue::moveUpQueue() {
    if (readyQueue.empty()) {
        runningItem = ReadyQueueItem{};
        return;
    }
    ReadyQueueItem newRunner = readyQueue.front();
    runningItem = newRunner;
    readyQueue.erase(readyQueue.begin());
}

std::vector<int> ReadyQueue::getQueue() {
    std::vector<int> queuePID = std::vector<int>();
    for (auto item : readyQueue) {
        queuePID.push_back(item.PID);
    }
    return queuePID;
}

void ReadyQueue::addToQueue(int PID, int priority) {
    // If runningPriority == -1, nothing is running AND in the queue
    ReadyQueueItem newItem = ReadyQueueItem{PID, priority};
    if (runningItem.priority == -1) {
        runningItem = newItem;
        return;
    }
    if (runningItem.priority < priority) {
        // Something running AND it has a lower priority
        // Make new PID running
        // Add last runner back to queue
        moveRunningToQueue(newItem);
        return;
    }

    int posToAdd = 0;
    for (posToAdd = 0; posToAdd < readyQueue.size(); posToAdd++) {
        ReadyQueueItem currentItem = readyQueue.at(posToAdd);
        if (priority > currentItem.priority) {
            break;
        }
    }
    readyQueue.insert(readyQueue.begin() + posToAdd, newItem);
}

int ReadyQueue::getRunningPID() {
    return runningItem.PID;
}

void ReadyQueue::moveRunningToQueue(ReadyQueueItem newRunner) {
    ReadyQueueItem temp = runningItem;
    addToQueue(temp.PID, temp.priority);
    runningItem = newRunner;
}

void ReadyQueue::removeFromQueue(int PID) {
    if (readyQueue.empty()) {
        return;
    }

    for (int i = 0; i < readyQueue.size(); i++) {
        int currentPID = readyQueue.at(i).PID;
        if (currentPID == PID) {
            readyQueue.erase(readyQueue.begin() + i);
        }
    }
}

