// Eric Vianney Sol Cruz

#include <algorithm>
#include <cmath>
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
    // Create new process
    int newPID = getNewPID();
    auto newItem = MemoryItem{availableAddr, size, newPID};
    Process newProcess = Process{newItem, priority};

    readyQueue.addToQueue(newPID, priority);
    processes[newPID] = newProcess;
    memUsage.push_back(newItem);

    // Sort by lowest to highest memory address
    std::sort(memUsage.begin(), memUsage.end(), compareMemAddress);
    return true;
}

bool SimOS::SimFork() {
    MemoryItem parentInfo = findPID(readyQueue.getRunningPID());
    int childPriority = processes[parentInfo.PID].priority;
    unsigned long long childSize = parentInfo.itemSize;
    bool created = NewProcess(childPriority, childSize);
    if (created) {
        processes[readyQueue.getRunningPID()].children.push_back(latestPID);
    }
    return created;
}

void SimOS::SimExit() {
    int parentPID = findParent(readyQueue.getRunningPID());
    int runningProcessPID = readyQueue.getRunningPID();
    if (parentPID) {
        if (processes[parentPID].waiting) {
            // Return waiting parent to ready queue
            processes[parentPID].waiting = false;
            readyQueue.addToQueue(processes[parentPID].memoryItem.PID, processes[parentPID].priority);
        } else {
            // Parent wasn't waiting, I'm a zombie
            processes[runningProcessPID].zombie = true;
        }
    }
    cascadeTerminate(readyQueue.getRunningPID());
    readyQueue.moveUpQueue();
}

void SimOS::SimWait() {
    int runningProcessPID = readyQueue.getRunningPID();
    Process running = processes[runningProcessPID];
    if (running.children.empty()) {
        readyQueue.addToQueue(running.memoryItem.PID, running.priority);
        readyQueue.moveUpQueue();
        return;
    }
    if (!running.children.empty()) {
        int killedZombie = findZombie(runningProcessPID);
        if (killedZombie > -1) {
            processes[runningProcessPID].children.erase(processes[runningProcessPID].children.begin() + killedZombie);
            readyQueue.addToQueue(running.memoryItem.PID, running.priority);
            readyQueue.moveUpQueue();
            return;
        }
    }
    processes[runningProcessPID].waiting = true;
    readyQueue.moveUpQueue();
}

void SimOS::DiskReadRequest(int diskNumber, std::string fileName) {
    int runningProcessPID = readyQueue.getRunningPID();
    disks.at(diskNumber).newRequest(runningProcessPID, fileName);
    processes[runningProcessPID].requestingDisk = diskNumber;
    readyQueue.moveUpQueue();
}

void SimOS::DiskJobCompleted(int diskNumber) {
    FileReadRequest runningRequest = disks.at(diskNumber).getRunningRequest();
    int finishedPID = runningRequest.PID;
    disks.at(diskNumber).moveUpQueue();
    processes[finishedPID].requestingDisk = -1;
    readyQueue.addToQueue(finishedPID, findPriority(finishedPID));
}

int SimOS::GetCPU() {
    return readyQueue.getRunningPID();
}

std::vector<int> SimOS::GetReadyQueue() {
    return readyQueue.getQueue();
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
        // The process size will make it go outside of ram range
        if ((possibleAddr + size) > amountOfRam) {
            return newAddr;
        }
        return item.itemAddress + item.itemSize;
    }
    // Check in between first item and start of memory
    if (memUsage.at(0).itemAddress > size) {
        smallestGapAddr = 0;
        smallestGapSize = memUsage.at(0).itemAddress;
    }
    int i = 0;
    for (i = 0; i < memUsage.size() - 1; i++) {
        MemoryItem current = memUsage.at(i);
        MemoryItem next = memUsage.at(i + 1);
        // Ending address has to be <= the next item address
        auto possibleStartingAddr = current.itemAddress + current.itemSize;
        auto possibleEndingAddr = possibleStartingAddr + size;
        auto gapSize = possibleEndingAddr - possibleStartingAddr;
        if (gapSize > smallestGapSize) {
            // We already have something better, go next
            continue;
        }
        if (next.itemAddress >= possibleEndingAddr && gapSize < smallestGapSize) {
            // The process will fit + the gap is smaller than the last one that was found.
            smallestGapSize = gapSize;
            smallestGapAddr = possibleStartingAddr;
        }
    }
    // Check last item if we didn't find anything
    if (smallestGapAddr > amountOfRam) {
        MemoryItem last = memUsage.at(memUsage.size() - 1);
        unsigned long long nextAddr = last.itemAddress + last.itemSize;
        if (nextAddr + size <= amountOfRam) {
            smallestGapAddr = nextAddr;
        }
    }
    // Returns out of ram range if there is no space
    return smallestGapAddr;
}

void SimOS::cascadeTerminate(int PID) {
    processes[PID].terminated = true;
    for (auto child : processes[PID].children) {
        // Terminate everything related to the process
        cascadeTerminate(processes[child].memoryItem.PID);
    }
    readyQueue.removeFromQueue(PID);
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
