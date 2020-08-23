//
// Created by 杜悦 on 2020/7/9.
//

#ifndef ASSIGNMENT1_WAITQUEUE_H
#define ASSIGNMENT1_WAITQUEUE_H

#include <list>
#include "task.h"
#include <climits>

class WaitQueue {
private:
    std::list<Task *> list;
    int maxSize;

public:
    WaitQueue(int maxSize) {
        this->maxSize = maxSize;
    }

    bool append(Task *node) {
        if (this->size() >= maxSize) {
            return false;
        }
        list.push_back(node);
        return true;
    }

    bool isFull() {
        return this->size() >= maxSize;
    }

    bool isEmpty() {
        return this->size() <= 0;
    }

    int size() {
        return list.size();
    }

    Task *pop(Task *currentNode) {
        int minDifference = INT_MAX;
        Task *perfectNode;
        auto itor = list.begin();
        while (itor != list.end()) {
            Task *node = *itor;
            if (abs(node->getRequestNum() - currentNode->getRequestNum()) < minDifference) {
                minDifference = abs(node->getRequestNum() - currentNode->getRequestNum());
                perfectNode = node;
            }
            itor++;
        }
        list.remove(perfectNode);
        return perfectNode;
    }

    int getMaxSize() const {
        return maxSize;
    }
};

#endif //ASSIGNMENT1_WAITQUEUE_H
