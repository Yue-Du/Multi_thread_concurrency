//
// Created by 杜悦 on 2020/7/14.
//

#ifndef ASSIGNMENT1_PRODUCER_H
#define ASSIGNMENT1_PRODUCER_H

#include "readfile.h"
#include "task.h"

class Producer {
    int id;
    ReadFile file;

public:
    Producer(int id, char *fileName) : file(fileName) {
        this->id = id;
    }

    Task *produceTask() {
        char line[100];
        if (!file.nextLine(line, 100) || strlen(line) == 0) {
            file.close();
            return NULL;
        }
        Task *task = new Task(atoi(line), id);
        return task;
    }

    int getId() const {
        return id;
    }

};

#endif //ASSIGNMENT1_PRODUCER_H
