//
// Created by 杜悦 on 2020/7/9.
//

#ifndef ASSIGNMENT1_TASK_H
#define ASSIGNMENT1_TASK_H

class Task {
    int requestNum;
    int producerId;
    bool executed;

public:
    Task(int requestNum, int fileId) {
        this->requestNum = requestNum;
        this->producerId = fileId;
        this->executed = false;
    }

    void setExecuted(bool executed) {
        Task::executed = executed;
    }

    int getRequestNum() const {
        return requestNum;
    }

    int getProducerId() const {
        return producerId;
    }

    bool isExecuted() const {
        return executed;
    }
};

#endif //ASSIGNMENT1_TASK_H
