// Eric Vianney Sol Cruz

#ifndef SIMOS_SIMOS_H
#define SIMOS_SIMOS_H

#include <string>
#include <vector>
#include <queue>
#include <map>
#include "DiskQueue.h"
#include "ReadyQueue.h"


struct MemoryItem {
    unsigned long long itemAddress;
    unsigned long long itemSize;
    int PID; // Process PID using this memory chunk
};

using MemoryUsage = std::vector<MemoryItem>;

struct Process {
    MemoryItem memoryItem = MemoryItem{0, 0, 0};
    int priority = -1;
    bool waiting = false;
    bool terminated = false;
    bool zombie = false;
    // Vector of children PIDs
    std::vector<int> children = std::vector<int>();
    int requestingDisk = -1;
};

class SimOS {
private:
    int latestPID = 0;
    unsigned long long amountOfRam;

    MemoryUsage memUsage;
    // key: PID, val: Process
    std::map<int, Process> processes;
    std::vector<Disk> disks;
    ReadyQueue readyQueue;

    int getNewPID();
    unsigned long long findAvailableMemory(unsigned long long size);
    void removeFromMemory(int PID);
    void cascadeTerminate(int PID);
    MemoryItem findPID(int PID);
    int findPriority(int PID);
    int findParent(int PID);
    void removeFromDisk(int PID);
    int findZombie(int PID);
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
};

#endif //SIMOS_SIMOS_H
