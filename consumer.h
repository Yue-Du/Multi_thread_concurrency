//
// Created by 杜悦 on 2020/7/14.
//

#ifndef ASSIGNMENT1_CONSUMER_H
#define ASSIGNMENT1_CONSUMER_H

#include "task.h"
#include <stdio.h>
#include <iostream>

using namespace std;

class Consumer {
public:
    void executetask(Task *task) {
        cout << "service requester " << task->getProducerId() << " track " << task->getRequestNum() << endl;
        task->setExecuted(true);
    }
};

#endif //ASSIGNMENT1_CONSUMER_H
