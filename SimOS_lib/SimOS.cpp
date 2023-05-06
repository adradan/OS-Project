// Eric Vianney Sol Cruz

#include <iostream>
#include <cmath>
#include "SimOS.h"


SimOS::SimOS(int numberOfDisks, unsigned long long int amountOfRam) {
    this->numberOfDisks = numberOfDisks;
    this->amountOfRam = amountOfRam;
    this->diskSize = amountOfRam / numberOfDisks;
    this->processMemory = std::vector<Process>();
    this->memoryUsage = MemoryUsage(numberOfDisks);
}

bool SimOS::NewProcess(int priority, unsigned long long size) {
    if (size <= 0 || size > amountOfRam) {
        return false;
    }

    bool hasSpace = false;
    unsigned long long currentSpace = 0;
    int smallestDiskNumber = 0;
    int currentDiskNumber = 0;

    for (auto disk : this->memoryUsage) {
        std::cout << "Current Disk: " << currentDiskNumber << std::endl;
        if (currentSpace >= size) {
            hasSpace = true;
            break;
        }
        if (disk.PID != 0) {
            // Encountered an existing process in the way, reset space counter
            currentSpace = 0;
            smallestDiskNumber = currentDiskNumber + 1;
        } else {
            currentSpace += this->diskSize;
        }
        currentDiskNumber++;
        std::cout << "Current Space: " << currentSpace << std::endl;
    }
    // In case the last disk provides enough space, we double-check the available space
    if (currentSpace >= size) {
        hasSpace = true;
    }
    if (!hasSpace) {
        return hasSpace;
    }

    unsigned long long newItemAddress = smallestDiskNumber * this->diskSize;
    int newPID = getNewPID();
    MemoryItem newItem = MemoryItem{newItemAddress, size, newPID};
    Process newProcess = Process{priority, newPID};

    this->readyQueue.addToReadyQueue(newProcess.priority, newItem);

    int disksToUse = std::ceil((double) size / (double) this->diskSize);
    for (int i = 0; i < disksToUse; i++) {
        this->memoryUsage[smallestDiskNumber + i] = newItem;
    }
    this->processMemory.push_back(newProcess);
    std::cout << "Smallest Disk: " << smallestDiskNumber << " PID: " << newItem.PID << " Disks Used: " << disksToUse << std::endl;
    return hasSpace;
}

bool SimOS::SimFork() {
    return false;
}

void SimOS::SimExit() {

}

void SimOS::SimWait() {

}

void SimOS::DiskReadRequest(int diskNumber, std::string fileName) {

}

void SimOS::DiskJobCompleted(int diskNumber) {

}

int SimOS::GetCPU() {
    int runningPID = memoryUsage.at(0).PID;
    if (runningPID == 0) {
        std::cout << "CPU is IDLE. Instruction ignored." << std::endl;
        return 0;
    }
    return runningPID;
}

std::vector<int> SimOS::GetReadyQueue() {
    std::vector<int> processPIDList;
    for (auto readyQueueItem : readyQueue.getQueue()) {
        if (readyQueueItem.item.PID == 0) {
            continue;
        }
        processPIDList.push_back(readyQueueItem.item.PID);
    }
    return processPIDList;
}

MemoryUsage SimOS::GetMemory() {
    MemoryUsage processesUsingMemory;
    for (auto process : processMemory) {
        if (process.PID == 0) {
            continue;
        }
        for (auto mem : memoryUsage) {
            if (mem.PID == process.PID) {
                processesUsingMemory.push_back(mem);
                break;
            }
        }
    }
    std::cout << "====" << std::endl;
    for (auto p : processesUsingMemory) {
        std::cout << p.itemAddress << std::endl;
    }
    std::cout << "====" << std::endl;
    return processesUsingMemory;
}

FileReadRequest SimOS::GetDisk(int diskNumber) {
    if (diskQueue.empty()) {
        return FileReadRequest();
    }
}

std::queue<FileReadRequest> SimOS::GetDiskQueue(int diskNumber) {
    return std::queue<FileReadRequest>();
}

int SimOS::getNumberOfDisks() const {
    return numberOfDisks;
}

unsigned long long int SimOS::getAmountOfRam() const {
    return amountOfRam;
}

unsigned long long int SimOS::getSizeOfDisk() const {
    return diskSize;
}

int SimOS::getNewPID() {
    this->latestPID = this->latestPID + 1;
    return this->latestPID;
}


// Ready Queue
void ReadyQueue::addToReadyQueue(int priority, MemoryItem item) {
    ReadyQueueItem newQueueItem = ReadyQueueItem{priority, item};
    int posToAdd = 0;

    for (auto itr = this->queue.begin(); itr != this->queue.end(); itr++, posToAdd++) {
        ReadyQueueItem currentItem = *itr;
        if (currentItem.priority > newQueueItem.priority) {
            break;
        }
    }
    this->queue.insert(this->queue.begin() + posToAdd, newQueueItem);
}

std::vector<ReadyQueueItem> ReadyQueue::getQueue() {
    return this->queue;
}
