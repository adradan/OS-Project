// Eric Vianney Sol Cruz

#include <iostream>
#include <cmath>
#include <unistd.h>
#include "SimOS.h"


SimOS::SimOS(int numberOfDisks, unsigned long long int amountOfRam) {
    this->numberOfDisks = numberOfDisks;
    this->amountOfRam = amountOfRam;
    diskSize = amountOfRam / numberOfDisks;
    processMemory = std::vector<Process>();
    numberOfPartitions = amountOfRam / PARTITION_SIZE;
    memoryUsage = MemoryUsage(numberOfPartitions, MemoryItem{});
    disks = std::vector<DiskQueue>(numberOfDisks, DiskQueue{});
}

bool SimOS::NewProcess(int priority, unsigned long long size) {
    if (size <= 0 || size > amountOfRam) {
        return false;
    }

    bool hasSpace = false;
    unsigned long long currentSpace = 0;
    int smallestPartitionNumber = 0;
    int currentPartitionNumber = 0;

    for (auto disk : memoryUsage) {
        std::cout << "Current Partition: " << currentPartitionNumber << std::endl;
        if (currentSpace >= size) {
            hasSpace = true;
            break;
        }
        if (disk.PID != 0) {
            // Encountered an existing process in the way, reset space counter
            currentSpace = 0;
            smallestPartitionNumber = currentPartitionNumber + 1;
        } else {
            currentSpace += PARTITION_SIZE;
        }
        currentPartitionNumber++;
        std::cout << "Current Space: " << currentSpace << std::endl;
    }
    // In case the last disk provides enough space, we double-check the available space
    if (currentSpace >= size) {
        hasSpace = true;
    }
    if (!hasSpace) {
        return hasSpace;
    }

    unsigned long long newItemAddress = smallestPartitionNumber * PARTITION_SIZE;
    int newPID = getNewPID();
    MemoryItem newItem = MemoryItem{newItemAddress, size, newPID};
    Process newProcess = Process{priority, newPID};

    this->readyQueue.addToReadyQueue(newProcess.priority, newProcess.PID);

    int partitionsToUse = std::ceil((double) size / (double) PARTITION_SIZE);
    for (int i = 0; i < partitionsToUse; i++) {
        this->memoryUsage[smallestPartitionNumber + i] = newItem;
    }
    this->processMemory.push_back(newProcess);
    std::cout << "Smallest Disk: " << smallestPartitionNumber << " PID: " << newItem.PID << " Partitions Used: " << partitionsToUse << std::endl;
    return hasSpace;
}

bool SimOS::SimFork() {
    pid_t childPid = fork();
    ReadyQueueItem currentQueueItem = readyQueue.getQueue().at(0);
    Process currentProcess = getProcess(currentQueueItem.PID);
    std::cout << processMemory.at(0).PID << std::endl;
    return true;
}

void SimOS::SimExit() {

}

void SimOS::SimWait() {

}

void SimOS::DiskReadRequest(int diskNumber, std::string fileName) {
    ReadyQueueItem processUsingCPU = readyQueue.getQueue().at(0);
    FileReadRequest newReadRequest = FileReadRequest{processUsingCPU.PID, fileName};

    readyQueue.removeFromQueue(processUsingCPU.PID);
    disks[diskNumber].push(newReadRequest);
}

void SimOS::DiskJobCompleted(int diskNumber) {
    Process foundProcess;
    for (auto process : processMemory) {
        if (process.PID == disks[diskNumber].front().PID) {
            foundProcess = process;
        }
    }

    if (readyQueue.getQueue().at(0).priority == foundProcess.priority) {
        readyQueue.addToReadyQueue(foundProcess.priority, foundProcess.PID, 0);
    } else {
        readyQueue.addToReadyQueue(foundProcess.priority, foundProcess.PID);
    }
    disks[diskNumber].pop();
}

int SimOS::GetCPU() {
    if (readyQueue.getQueue().empty()) {
        std::cout << "CPU is IDLE. Instruction ignored." << std::endl;
        return 0;
    }
    int runningPID = readyQueue.getQueue().at(0).PID;
    return runningPID;
}

std::vector<int> SimOS::GetReadyQueue() {
    std::vector<int> processPIDList;
    for (auto readyQueueItem : readyQueue.getQueue()) {
        if (readyQueueItem.PID == 0) {
            continue;
        }
        processPIDList.push_back(readyQueueItem.PID);
    }
    return processPIDList;
}

MemoryUsage SimOS::GetMemory() {
    MemoryUsage processesUsingMemory;
    MemoryItem lastFoundProcess{};
    for (auto mem : memoryUsage) {
        if (mem.PID != 0 && mem.PID != lastFoundProcess.PID) {
            lastFoundProcess = mem;
            processesUsingMemory.push_back(mem);
        }
    }
    if (processesUsingMemory.empty()) {
        std::cout << "CPU is idle. Instruction ignored." << std::endl;
    }
    return processesUsingMemory;
}

FileReadRequest SimOS::GetDisk(int diskNumber) {
    DiskQueue currentQueue = disks.at(diskNumber);
    return currentQueue.front();
}

std::queue<FileReadRequest> SimOS::GetDiskQueue(int diskNumber) {
    return disks.at(diskNumber);
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

Process SimOS::getProcess(int PID) {
    for (auto process : processMemory) {
        if (process.PID == PID) {
            return process;
        }
    }
    return Process{};
}

// Ready Queue
void ReadyQueue::addToReadyQueue(int priority, int PID) {
    ReadyQueueItem newQueueItem = ReadyQueueItem{priority, PID};
    if (queue.empty()) {
        queue.insert(queue.begin(), newQueueItem);
        return;
    }

    for (int i = 0; i < queue.size(); i++) {
        ReadyQueueItem currentItem = queue.at(i);
        if (currentItem.priority > newQueueItem.priority) {
            queue.insert(queue.begin() + i, newQueueItem);
            break;
        }
    }
}

void ReadyQueue::addToReadyQueue(int priority, int PID, int pos) {
    ReadyQueueItem newQueueItem = ReadyQueueItem{priority, PID};
    if (pos < 0 || pos > queue.size()) {
        return;
    }
    queue.insert(queue.begin() + pos, newQueueItem);
}

std::vector<ReadyQueueItem> ReadyQueue::getQueue() {
    return this->queue;
}

void ReadyQueue::removeFromQueue(int PID) {
    for (int i = 0; i < queue.size(); i++) {
        ReadyQueueItem queueItem = queue.at(i);
        if (queueItem.PID == PID) {
            queue.erase(queue.begin() + i);
        }
    }
}