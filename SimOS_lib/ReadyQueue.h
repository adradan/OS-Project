//
// Eric Vianney Sol Cruz
//

#ifndef SIMOS_READYQUEUE_H
#define SIMOS_READYQUEUE_H


#include <vector>

struct ReadyQueueItem {
    int PID = 0;
    int priority = -1;
};

class ReadyQueue {
private:
    std::vector<ReadyQueueItem> readyQueue = std::vector<ReadyQueueItem>();
    ReadyQueueItem runningItem = ReadyQueueItem{};
    void moveRunningToQueue(ReadyQueueItem newRunner);
public:
    void moveUpQueue();
    std::vector<int> getQueue();
    void addToQueue(int PID, int priority);
    int getRunningPID();
    void removeFromQueue(int PID);
};


#endif //SIMOS_READYQUEUE_H
