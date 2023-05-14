//
// Eric Vianney Sol Cruz
//

#ifndef SIMOS_DISKQUEUE_H
#define SIMOS_DISKQUEUE_H

#include <queue>
#include <string>

struct FileReadRequest {
    int PID{0};
    std::string fileName{""};
};

using DiskQueue = std::queue<FileReadRequest>;

class Disk {
private:
    DiskQueue diskQueue = std::queue<FileReadRequest>();
    FileReadRequest runningRequest = FileReadRequest();
    void addToQueue(FileReadRequest request);
public:
    void moveUpQueue();
    DiskQueue getQueue();
    FileReadRequest getRunningRequest();
    void removeFromDisk(int PID);
    void newRequest(int PID, std::string fileName);
};


#endif //SIMOS_DISKQUEUE_H
