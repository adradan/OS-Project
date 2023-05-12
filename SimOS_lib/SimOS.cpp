// Eric Vianney Sol Cruz

#include <iostream>
#include <cmath>
#include <unistd.h>
#include "SimOS.h"

SimOS::SimOS(int numberOfDisks, unsigned long long int amountOfRam) {
    this->numberOfDisks = numberOfDisks;
    this->amountOfRam = amountOfRam;
    this->disks = std::vector<Disk>(numberOfDisks, Disk());
}

bool SimOS::NewProcess(int priority, unsigned long long int size) {
    if (size > amountOfRam) {
        return false;
    }
    // Find space
    unsigned long long availableAddr = findAvailableMemory(size);
    if (availableAddr > amountOfRam) {
        return false;
    }
    // Get PID
    int newPID = getNewPID();
    auto newItem = MemoryItem{availableAddr, size, newPID};
    priorities[newPID] = priority;
    addToReadyQueue(newPID, priority);
    memUsage.push_back(newItem);
    return true;
}

bool SimOS::SimFork() {
    return false;
}

void SimOS::SimExit() {

}

void SimOS::SimWait() {

}

void SimOS::DiskReadRequest(int diskNumber, std::string fileName) {
    auto newRequest = FileReadRequest{runningProcess, fileName};
    Disk disk = disks.at(diskNumber);
    if (disk.queue.empty() && disk.runningRequest.PID == 0) {
        disks.at(diskNumber).runningRequest = newRequest;
    } else {
        disks.at(diskNumber).queue.push(newRequest);
    }
    if (readyQueue.empty()) {
        runningProcess = 0;
    } else {
        runningProcess = readyQueue.at(0);
        readyQueue.erase(readyQueue.begin());
    }
}

void SimOS::DiskJobCompleted(int diskNumber) {
    Disk disk = disks.at(diskNumber);
    FileReadRequest finishedJob = disk.runningRequest;
    int finishedPID = finishedJob.PID;
    FileReadRequest nextRequest = FileReadRequest{};
    if (!disk.queue.empty()) {
        nextRequest = disk.queue.front();
        disks.at(diskNumber).queue.pop();
    }
    disks.at(diskNumber).runningRequest = nextRequest;
    addToReadyQueue(finishedPID, priorities[finishedPID]);
}

int SimOS::GetCPU() {
    return runningProcess;
}

std::vector<int> SimOS::GetReadyQueue() {
    return readyQueue;
}

MemoryUsage SimOS::GetMemory() {
    return MemoryUsage();
}

FileReadRequest SimOS::GetDisk(int diskNumber) {
    return disks.at(diskNumber).runningRequest;
}

std::queue<FileReadRequest> SimOS::GetDiskQueue(int diskNumber) {
    return disks.at(diskNumber).queue;
}

int SimOS::getNewPID() {
    latestPID += 1;
    return latestPID;
}

void SimOS::addToReadyQueue(int PID, int priority) {
    int runningPriority = priorities[runningProcess];
    if (runningPriority < priority) {
        swapRunningProcess(PID);
        return;
    }
    if (readyQueue.empty()) {
        if (runningProcess != 0) {
            if (runningPriority < priority) {
                swapRunningProcess(PID);
                return;
            }
        } else {
            runningProcess = PID;
            return;
        }
    }
    int posToAdd = 0;
    for (posToAdd = 0; posToAdd < readyQueue.size(); posToAdd++) {
        int currentPID = readyQueue.at(posToAdd);
        int currentPriority = priorities[currentPID];
        if (priority > currentPriority) {
            break;
        }
    }
    readyQueue.insert(readyQueue.begin() + posToAdd, PID);
}

unsigned long long SimOS::findAvailableMemory(unsigned long long size) {
    if (memUsage.empty()) {
        return 0;
    }
    // Make sure memory usage has at least 2 processes.
    if (memUsage.size() == 1) {
        MemoryItem item = memUsage.at(0);
        return item.itemAddress + item.itemSize;
    }
    bool foundSpace = false;
    unsigned long long newAddr = amountOfRam + 1;
    int i = 0;
    for (i = 0; i < memUsage.size() - 1; i++) {
        if (foundSpace) {
            break;
        }
        MemoryItem current = memUsage.at(i);
        MemoryItem next = memUsage.at(i + 1);
        auto possibleStartingAddr = current.itemAddress + current.itemSize;
        auto possibleEndingAddr = possibleStartingAddr + size;
        if (next.itemAddress > possibleStartingAddr) {
            // There's a hole in the memory, let's check if the new process will fit in it
            if (next.itemAddress >= possibleEndingAddr) {
                foundSpace = true;
                newAddr = possibleStartingAddr;
            }
        }
    }
    // Check last item if we didn't find anything
    if (!foundSpace) {
        MemoryItem last = memUsage.at(memUsage.size() - 1);
        unsigned long long nextAddr = last.itemAddress + last.itemSize;
        if (nextAddr < amountOfRam) {
            if (nextAddr + size <= amountOfRam) {
                foundSpace = true;
                newAddr = nextAddr;
            }
        }
    }
    // Returns out of ram range if no space
    return newAddr;
}

void SimOS::swapRunningProcess(int newPID) {
    int runningPriority = priorities[runningProcess];
    int temp = runningProcess;
    addToReadyQueue(temp, runningPriority);
    runningProcess = newPID;
}
