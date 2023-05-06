// Eric Vianney Sol Cruz

#ifndef SIMOS_SIMOS_H
#define SIMOS_SIMOS_H

#include <string>
#include <vector>
#include <queue>
#include "gtest/gtest.h"

struct FileReadRequest {
    int PID{0};
    std::string fileName{""};
};

struct MemoryItem {
    unsigned long long itemAddress;
    unsigned long long itemSize;
    int PID; // Process PID using this memory chunk
};

struct ReadyQueueItem {
    int priority;
    int PID;
};

using MemoryUsage = std::vector<MemoryItem>;

struct Process {
    int priority;
    int PID{0};
};

using DiskQueue = std::queue<FileReadRequest>;

class ReadyQueue {
private:
    std::vector<ReadyQueueItem> queue;
public:
    void addToReadyQueue(int priority, int PID);
    void addToReadyQueue(int priority, int PID, int pos);
    void removeFromQueue(int PID);
    std::vector<ReadyQueueItem> getQueue();
};

class SimOS {
private:
    int latestPID = 0;
    int numberOfDisks;
    int numberOfPartitions;
    unsigned long long amountOfRam;
    unsigned long long diskSize;

    static const int PARTITION_SIZE = 4;

    // Process Memory and Memory Usage have a 1:Many relationship
    // 1 Process can use many disks
    std::vector<Process> processMemory;
    MemoryUsage memoryUsage;
    ReadyQueue readyQueue;
    std::vector<DiskQueue> disks;

    int getNewPID();
public:
    SimOS(int numberOfDisks, unsigned long long amountOfRam);
    bool NewProcess(int priority, unsigned long long size);
    bool SimFork();
    void SimExit();
    void SimWait();
    void DiskReadRequest(int diskNumber, std::string fileName);
    void DiskJobCompleted(int diskNumber);
    int GetCPU();
    std::vector<int> GetReadyQueue();
    MemoryUsage GetMemory();
    FileReadRequest GetDisk(int diskNumber);
    std::queue<FileReadRequest> GetDiskQueue(int diskNumber);

    int getNumberOfDisks() const;

    unsigned long long int getAmountOfRam() const;

    unsigned long long int getSizeOfDisk() const;
};

#endif //SIMOS_SIMOS_H
