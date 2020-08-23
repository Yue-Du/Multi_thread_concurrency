//
// Created by 杜悦 on 2020/7/14.
//

#include <stdio.h>
#include "thread.h"
#include <iostream>
#include <algorithm>
#include "task.h"
#include "waitqueue.h"
#include "consumer.h"
#include "producer.h"

using namespace std;

typedef struct ProducerParam {
    char *fileName;
    int id;
} ProducerParam;

WaitQueue *waitQueue;
int activeProducer;

void producerThreadFunction(void *value) {
    ProducerParam *param = (ProducerParam *) value;
    Producer *producer = new Producer(param->id, param->fileName);
    Task *task = NULL;
    thread_lock(1);
    while (task = producer->produceTask()) {
        while (waitQueue->isFull()) {
            thread_wait(1, 1);
        }
        waitQueue->append(task);
        cout << "requester " << producer->getId() << " track " << task->getRequestNum() << endl;
        thread_broadcast(1, 1);
        while (!task->isExecuted()) {
            thread_wait(1, producer->getId());
        }
    }
    activeProducer--;
    thread_broadcast(1, 1);
    thread_unlock(1);
}

void consumerThreadFunction(void *value) {
    Consumer *consumer = new Consumer();
    thread_lock(1);
    Task *task = NULL;
    while ((activeProducer > 0) or (!waitQueue->isEmpty())) {
        while ((activeProducer > waitQueue->getMaxSize() && !waitQueue->isFull()) ||
               activeProducer < waitQueue->getMaxSize()) {
            thread_wait(1, 1);
        }
        task = waitQueue->pop(task);
        consumer->executetask(task);
        thread_broadcast(1, 1);
        thread_signal(1, task->getProducerId());
    }
    thread_unlock(1);
}

void first_thread_start_function(void *value) {
    start_preemptions(true, true, 1);
    int file_cnt = *(int *) value;
    printf("file_cnt: %d \n", file_cnt);
    for (int i = 0; i < file_cnt; i++) {
        ProducerParam *param = new ProducerParam();
        param->fileName = (*((char **) ((int *) value + 2) + i));
        param->id = i;
        thread_create(producerThreadFunction, (void *) param);
    }
    thread_create(consumerThreadFunction, NULL);
}

int main(int argc, char *argv[]) {
    printf("argc: %d \n", argc);
    for (int i = 0; i < argc; i++) {
        printf("argv %d: %s \n", i, argv[i]);
    }
    waitQueue = new WaitQueue(atoi(argv[1]));
    activeProducer = argc - 2;
    void *para = malloc(argc * sizeof(unsigned long) + sizeof(int) * 2);
    (*(int *) para) = argc - 2;
    (*((int *) para + 1)) = atoi(argv[1]);
    for (int i = 1; i < argc - 1; i++) {
        (*((char **) ((int *) para + 2) + i - 1)) = argv[i + 1];
    }
    thread_libinit(first_thread_start_function, para);
    return 0;
}
