// Eric Vianney Sol Cruz

#include <iostream>
#include <algorithm>
#include <cmath>
#include <unistd.h>
#include "SimOS.h"

bool compareMemAddress(MemoryItem item1, MemoryItem item2);

SimOS::SimOS(int numberOfDisks, unsigned long long int amountOfRam) {
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
    Process newProcess = Process{newItem, priority};
    addToReadyQueue(newPID, priority);
    processes[newPID] = newProcess;
    memUsage.push_back(newItem);
    std::sort(memUsage.begin(), memUsage.end(), compareMemAddress);
    return true;
}

bool SimOS::SimFork() {
    MemoryItem parentInfo = findPID(runningProcessPID);
    int childPriority = processes[parentInfo.PID].priority;
    unsigned long long childSize = parentInfo.itemSize;
    bool created = NewProcess(childPriority, childSize);
    if (created) {
        processes[runningProcessPID].children.push_back(latestPID);
    }
    return created;
}

void SimOS::SimExit() {
    int parentPID = findParent(runningProcessPID);
    if (parentPID) {
        if (processes[parentPID].waiting) {
            processes[parentPID].waiting = false;
            addToReadyQueue(processes[parentPID].memoryItem.PID, processes[parentPID].priority);
        } else {
            processes[runningProcessPID].zombie = true;
        }
    }
    cascadeTerminate(runningProcessPID);
    moveUpReadyQueue();
}

void SimOS::SimWait() {
    Process running = processes[runningProcessPID];
    if (running.children.empty()) {
        addToReadyQueue(running.memoryItem.PID, running.priority);
        moveUpReadyQueue();
        return;
    }
    if (!running.children.empty()) {
        int killedZombie = findZombie(runningProcessPID);
        if (killedZombie > -1) {
            processes[runningProcessPID].children.erase(processes[runningProcessPID].children.begin() + killedZombie);
            addToReadyQueue(running.memoryItem.PID, running.priority);
            moveUpReadyQueue();
            return;
        }
    }
    processes[runningProcessPID].waiting = true;
    moveUpReadyQueue();
}

void SimOS::DiskReadRequest(int diskNumber, std::string fileName) {
    disks.at(diskNumber).newRequest(runningProcessPID, fileName);
    processes[runningProcessPID].requestingDisk = diskNumber;
    int newRunner = readyQueue.moveUpQueue();
    runningProcessPID = newRunner;
//    if (readyQueue.empty()) {
//        runningProcessPID = 0;
//    } else {
//        moveUpReadyQueue();
//    }
}

void SimOS::DiskJobCompleted(int diskNumber) {
    FileReadRequest runningRequest = disks.at(diskNumber).getRunningRequest();
    int finishedPID = runningRequest.PID;
    disks.at(diskNumber).moveUpQueue();
    processes[finishedPID].requestingDisk = -1;
    addToReadyQueue(finishedPID, findPriority(finishedPID));
}

int SimOS::GetCPU() {
    return runningProcessPID;
}

std::vector<int> SimOS::GetReadyQueue() {
    return readyQueue;
}

MemoryUsage SimOS::GetMemory() {
    return memUsage;
}

FileReadRequest SimOS::GetDisk(int diskNumber) {
    return disks.at(diskNumber).getRunningRequest();
}

std::queue<FileReadRequest> SimOS::GetDiskQueue(int diskNumber) {
    return disks.at(diskNumber).getQueue();
}

int SimOS::getNewPID() {
    latestPID += 1;
    return latestPID;
}

void SimOS::addToReadyQueue(int PID, int priority) {
    int runningPriority = findPriority(runningProcessPID);
    if (runningPriority < priority) {
        swapRunningProcess(PID);
        return;
    }
    if (readyQueue.empty()) {
        if (runningProcessPID != 0) {
            if (runningPriority < priority) {
                swapRunningProcess(PID);
                return;
            }
        } else {
            runningProcessPID = PID;
            return;
        }
    }
    int posToAdd = 0;
    for (posToAdd = 0; posToAdd < readyQueue.size(); posToAdd++) {
        int currentPID = readyQueue.at(posToAdd);
        int currentPriority = findPriority(currentPID);
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
    unsigned long long smallestGapAddr = amountOfRam + 1;
    unsigned long long smallestGapSize = 0;
    unsigned long long newAddr = amountOfRam + 1;
    // Make sure memory usage has at least 2 processes.
    if (memUsage.size() == 1) {
        MemoryItem item = memUsage.at(0);
        unsigned long long possibleAddr = item.itemAddress + item.itemSize;
        if ((possibleAddr + size) > amountOfRam) {
            return newAddr;
        }
        return item.itemAddress + item.itemSize;
    }
    // Check first item
    if (memUsage.at(0).itemAddress > size) {
        smallestGapAddr = 0;
        smallestGapSize = memUsage.at(0).itemAddress;
    }
    int i = 0;
    for (i = 0; i < memUsage.size() - 1; i++) {
        MemoryItem current = memUsage.at(i);
        MemoryItem next = memUsage.at(i + 1);
        auto possibleStartingAddr = current.itemAddress + current.itemSize;
        auto possibleEndingAddr = possibleStartingAddr + size;
        auto gapSize = possibleEndingAddr - possibleStartingAddr;
        if (gapSize > smallestGapSize) {
            continue;
        }
        if (next.itemAddress > possibleStartingAddr) {
            // There's a hole in the memory, let's check if the new process will fit in it
            if (next.itemAddress >= possibleEndingAddr && gapSize < smallestGapSize) {
                smallestGapSize = gapSize;
                smallestGapAddr = possibleStartingAddr;
            }
        }
    }
    // Check last item if we didn't find anything
    if (smallestGapAddr > amountOfRam) {
        MemoryItem last = memUsage.at(memUsage.size() - 1);
        unsigned long long nextAddr = last.itemAddress + last.itemSize;
        if (nextAddr < amountOfRam) {
            if (nextAddr + size <= amountOfRam) {
                smallestGapAddr = nextAddr;
            }
        }
    }
    // Returns out of ram range if no space
    return smallestGapAddr;
}

void SimOS::swapRunningProcess(int newPID) {
    int runningPriority = findPriority(runningProcessPID);
    int temp = runningProcessPID;
    addToReadyQueue(temp, runningPriority);
    runningProcessPID = newPID;
}

void SimOS::moveUpReadyQueue() {
    if (readyQueue.empty()) {
        runningProcessPID = 0;
        return;
    }
    int newRunner = readyQueue.front();
    runningProcessPID = newRunner;
    readyQueue.erase(readyQueue.begin());
}

void SimOS::cascadeTerminate(int PID) {
    processes[PID].terminated = true;
    removeFromReadyQueue(PID);
    for (auto child : processes[PID].children) {
        cascadeTerminate(processes[child].memoryItem.PID);
    }
    removeFromMemory(PID);
    removeFromDisk(PID);
    processes[PID].requestingDisk = -1;
}

MemoryItem SimOS::findPID(int PID) {
    for (auto item : memUsage) {
        if (item.PID == PID) {
            return item;
        }
    }
    return MemoryItem{0, 0, 0};
}

int SimOS::findPriority(int PID) {
    return processes[PID].priority;
}

int SimOS::findParent(int PID) {
    for (auto p : processes) {
        Process process = p.second;
        if (std::find(process.children.begin(), process.children.end(), PID) != process.children.end()) {
            return process.memoryItem.PID;
        }
    }
    return 0;
}

void SimOS::removeFromReadyQueue(int PID) {
    if (readyQueue.empty()) {
        return;
    }
    for (int i = 0; i < readyQueue.size(); i++) {
        int currentPID = readyQueue.at(i);
        if (currentPID == PID) {
            readyQueue.erase(readyQueue.begin() + i);
        }
    }
}

void SimOS::removeFromMemory(int PID) {
    int posToRemove = 0;
    for (posToRemove = 0; posToRemove < memUsage.size(); posToRemove++) {
        MemoryItem mem = memUsage.at(posToRemove);
        if (mem.PID == PID) {
            break;
        }
    }
    memUsage.erase(memUsage.begin() + posToRemove);
}

bool compareMemAddress(MemoryItem item1, MemoryItem item2) {
    return (item1.itemAddress < item2.itemAddress);
}

void SimOS::removeFromDisk(int PID) {
    if (disks.empty()) {
        return;
    }
    Process process = processes[PID];
    if (process.requestingDisk == -1) {
        return;
    }
    disks.at(process.requestingDisk).removeFromDisk(PID);
}

int SimOS::findZombie(int PID) {
    Process process = processes[PID];
    for (int i = 0; i < process.children.size(); i++) {
        int childPID = process.children.at(i);
        Process child = processes[childPID];
        if (child.zombie) {
            return i;
        }
    }
    return -1;
}
