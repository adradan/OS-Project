// Eric Vianney Sol Cruz

#ifndef SIMOS_SIMOS_H
#define SIMOS_SIMOS_H

#include <string>
#include <vector>
#include <queue>
#include <map>
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

using MemoryUsage = std::vector<MemoryItem>;

struct Process : public MemoryItem {
    int priority;
};

using ProcessUsage = std::vector<Process>;

using DiskQueue = std::queue<FileReadRequest>;

struct Disk {
    DiskQueue queue = std::queue<FileReadRequest>();
    FileReadRequest runningRequest = FileReadRequest();
};

class SimOS {
private:
    int latestPID = 0;
    int numberOfDisks;
    unsigned long long amountOfRam;
    int runningProcess = 0;

    MemoryUsage memUsage;
    // key: PID, val: priority
    std::map<int, int> priorities;
    std::vector<Disk> disks;
    std::vector<int> readyQueue;
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

    int getNewPID();
    void addToReadyQueue(int PID, int priority);
    unsigned long long findAvailableMemory(unsigned long long size);
    void swapRunningProcess(int newPID);
};

#endif //SIMOS_SIMOS_H
