//
// Created by h3yb4ws on 5/5/2023.
//
#include "gtest/gtest.h"
#include "SimOS.h"

class ReadyQueueFixture : public ::testing::Test {
protected:
    virtual void SetUp() {
        queue = new ReadyQueue();
        genericItem = MemoryItem{0, 20, 1};
    }

    virtual void TearDown() {
        delete queue;
        genericItem = MemoryItem{0, 20, 1};
    }

    ReadyQueue* queue;
    MemoryItem genericItem;
};

int getReadyQueuePos(int PID, ReadyQueue* queue) {
    int i = 0;
    auto q = queue->getQueue();
    for (i = 0; i < q.size(); i++) {
        if (q.at(i).PID == PID) {
            break;
        }
    }
    return i;
}

TEST_F(ReadyQueueFixture, AddOneReadyQueue) {
    queue->addToReadyQueue(1, genericItem.PID);

    EXPECT_EQ(queue->getQueue().size(), 1);
}

TEST_F(ReadyQueueFixture, AddInBtwnSamePriorityQueue) {
    queue->addToReadyQueue(1, genericItem.PID);
    genericItem.PID = 2;
    queue->addToReadyQueue(2, genericItem.PID);
    genericItem.PID = 3;
    queue->addToReadyQueue(1, genericItem.PID);

    int queuePos = getReadyQueuePos(3, queue);
    EXPECT_EQ(queuePos, 1);
}

TEST_F(ReadyQueueFixture, AddHigherPriority) {
    queue->addToReadyQueue(2, genericItem.PID);
    genericItem.PID = 2;
    queue->addToReadyQueue(1, genericItem.PID);
    int queuePos = getReadyQueuePos(2, queue);
    EXPECT_EQ(queuePos, 0);
}

TEST_F(ReadyQueueFixture, AddLowerPriority) {
    queue->addToReadyQueue(1, genericItem.PID);
    genericItem.PID = 2;
    queue->addToReadyQueue(2, genericItem.PID);
    int queuePos = getReadyQueuePos(2, queue);
    EXPECT_EQ(queuePos, 1);
}

class SimOSFixture : public ::testing::Test {
protected:
    virtual void SetUp() {
        os = new SimOS(5, 100);
        genericItem = MemoryItem{0, 20, 1};
    }

    virtual void TearDown() {
        delete os;
        genericItem = MemoryItem{0, 20, 1};
    }

    SimOS* os;
    MemoryItem genericItem;
};

TEST_F(SimOSFixture, NewProcessSuccessUnderDiskSize) {
    bool created = os->NewProcess(1, 10);
    EXPECT_EQ(created, true);
}

TEST_F(SimOSFixture, NewProcessSuccessOverDiskSize) {
    bool created = os->NewProcess(1, 30);
    EXPECT_EQ(created, true);
}

TEST_F(SimOSFixture, NewProcessFailOverTotal) {
    bool created = os->NewProcess(1, 200);
    EXPECT_EQ(created, false);
}

TEST_F(SimOSFixture, NewProcessFailZero) {
    bool created = os->NewProcess(1, 0);
    EXPECT_EQ(created, false);
}

TEST_F(SimOSFixture, MemReturnsOnePID) {
    os->NewProcess(1, 20);
    auto mem = os->GetMemory();
    for (auto m : mem) {
        std::cout << m.PID << std::endl;
    }
    EXPECT_EQ(mem[0].PID, 1);
    EXPECT_EQ(mem.size(), 1);
}

TEST_F(SimOSFixture, MemReturnsProperAddresses) {
    os->NewProcess(1, 20);
    os->NewProcess(1, 20);
    os->NewProcess(1, 20);
    auto mem = os->GetMemory();
    EXPECT_EQ(mem[0].itemAddress, 0);
    EXPECT_EQ(mem[1].itemAddress, 20);
    EXPECT_EQ(mem[2].itemAddress, 40);

    EXPECT_EQ(mem[0].PID, 1);
    EXPECT_EQ(mem[1].PID, 2);
    EXPECT_EQ(mem[2].PID, 3);
}

TEST_F(SimOSFixture, GetEmptyMemory) {
    auto mem = os->GetMemory();
    EXPECT_EQ(mem.empty(), true);
}

TEST_F(SimOSFixture, MemReturnsProperAddressesIrregularSize) {
    os->NewProcess(1, 20);
    // Will use 1 extra partition (total of 8)
    os->NewProcess(1, 30);
    os->NewProcess(1, 20);
    auto mem = os->GetMemory();
    EXPECT_EQ(mem[0].itemAddress, 0);
    EXPECT_EQ(mem[1].itemAddress, 20);
    EXPECT_EQ(mem[2].itemAddress, 52);

    EXPECT_EQ(mem[0].PID, 1);
    EXPECT_EQ(mem[1].PID, 2);
    EXPECT_EQ(mem[2].PID, 3);
}

TEST_F(SimOSFixture, GetIdleCPU) {
    int x = os->GetCPU();
    EXPECT_EQ(x, 0);
}

TEST_F(SimOSFixture, GetWorkingCPU) {
    os->NewProcess(1, 30);
    int x = os->GetCPU();
    EXPECT_EQ(x, 1);
}

TEST_F(SimOSFixture, ReadyQueueEmpty) {
    auto queue = os->GetReadyQueue();
    EXPECT_EQ(queue.empty(), true);
}

TEST_F(SimOSFixture, ReadyQueueOneItem) {
    os->NewProcess(1, 20);
    auto queue = os->GetReadyQueue();
    EXPECT_EQ(queue.size(), 1);
    EXPECT_EQ(queue.at(0), 1);
}

TEST_F(SimOSFixture, ReadyQueueReturnsInPriorityOrder) {
    os->NewProcess(1, 20);
    os->NewProcess(2, 20);
    auto queue = os->GetReadyQueue();
    for (int i = 0; i < queue.size(); i++) {
        EXPECT_EQ(queue.at(i), i + 1);
    }
}

TEST_F(SimOSFixture, ReadyQueueReturnsPriorityOrderUnorderedAdd) {
    os->NewProcess(2, 20);
    os->NewProcess(1, 20);

    auto queue = os->GetReadyQueue();
    EXPECT_EQ(queue.at(0), 2);
    EXPECT_EQ(queue.at(1), 1);
}

TEST_F(SimOSFixture, Fork) {
    os->NewProcess(1, 20);
    os->SimFork();
    EXPECT_EQ(true, true);
}

