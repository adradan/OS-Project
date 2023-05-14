//
// Eric Vianney Sol Cruz
//

#include "DiskQueue.h"

DiskQueue Disk::getQueue() {
    return diskQueue;
}

FileReadRequest Disk::getRunningRequest() {
    return runningRequest;
}


void Disk::moveUpQueue() {
    if (this->diskQueue.empty()) {
        runningRequest = FileReadRequest();
        return;
    }
    FileReadRequest nextRequest = diskQueue.front();
    runningRequest = nextRequest;
    diskQueue.pop();
}

void Disk::removeFromDisk(int PID) {
    // PID is the current request
    if (runningRequest.PID == PID) {
        moveUpQueue();
        return;
    }

    // Check if PID is in the queue
    DiskQueue newQueue;
    DiskQueue currentQueue = diskQueue;

    while (!currentQueue.empty() && currentQueue.front().PID != PID) {
        newQueue.push(currentQueue.front());
        currentQueue.pop();
    }

    // No matching PID in disk
    if (currentQueue.empty()) {
        return;
    }

    // Found the PID
    currentQueue.pop();
    while (!currentQueue.empty()) {
        newQueue.push(currentQueue.front());
        currentQueue.pop();
    }
    diskQueue = newQueue;
}

void Disk::newRequest(int PID, std::string fileName) {
    FileReadRequest newRequest = FileReadRequest{PID, fileName};
    addToQueue(newRequest);
}

void Disk::addToQueue(FileReadRequest request) {
    diskQueue.push(request);
    if (diskQueue.size() == 1 && runningRequest.PID == 0) {
        moveUpQueue();
    }
}
