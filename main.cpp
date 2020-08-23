#include <stdio.h>
#include "thread.h"
#include<string.h>
#include <iostream>
#include <algorithm>
#include "task.h"
#include "waitqueue.h"

using namespace std;

WaitQueue *wait_queue = new WaitQueue();
int max_size;
int thread_num = -1;
Task *cur_track = new Task(0, -1, 0);
int active_producer;

void ReadTxt(void *filename) {
    char data[100];
    FILE *fp = fopen((char *) filename, "r");
    if (!fp) {
        printf("can't open file\n");
    }
    thread_num++;
    //active_producer++;
    int cur_thread = thread_num;
    while (!feof(fp)) {
        data[0] = '\0';
        fgets(data, sizeof(data), fp);
        if (strlen(data) == 0) {
            break;
        }
        if (data[strlen(data) - 1] == '\n') {
            data[strlen(data) - 1] = '\0';
        }

        thread_lock(1);
        while (wait_queue->size() >= max_size) {
            thread_wait(1, 1);
        }
        Task *pTask = new Task(atoi(data), cur_thread, 0);
        wait_queue->append(pTask);
        cout << "requester " << cur_thread << " track " << data << endl;
        thread_broadcast(1, 1);
        while(pTask->getFileState() == 0){
            thread_wait(1, cur_thread);
        }
        thread_unlock(1);

    }
    fclose(fp);
    thread_lock(1);
    active_producer--;
    thread_broadcast(1,1);
    thread_unlock(1);
    //cout << "request"<< cur_thread << "out\n";
}

void ServiceThread(void *pVoid) {
    thread_lock(1);
    while ((active_producer > 0) or (wait_queue->size() > 0)) {
        if (active_producer > max_size){
            while (wait_queue->size() < max_size) {
                //cout << "service wait 1\n";
                thread_wait(1, 1);
                //cout << "service wake up 1\n";
            }
        }
        else{
            while (wait_queue->size() < active_producer) {
                //cout << "service wait 2\n";
                thread_wait(1, 1);
                //cout << "service wake up 2\n";
            }
        }
        if(active_producer ==0){
            return;
        }
        Task *cur = wait_queue->pop(cur_track);
        cout << "service requester " << cur->getProducerId() << " track " << cur->getRequestNum() << endl;
        cur_track = cur;
        cur->setFileState(1);
        thread_broadcast(1, 1);
        thread_signal(1, cur->getProducerId());
    }
    thread_unlock(1);
    //cout << "service out\n";
}

void first_thread_start_function(void *value) {
    start_preemptions(true, true, 1);
    int file_cnt = *(int *) value;
    printf("file_cnt: %d \n", file_cnt);
    for (int i = 0; i < file_cnt; i++) {
        thread_create(ReadTxt, (*((char **) ((int *) value + 2) + i)));
    }
    thread_create(ServiceThread, NULL);
}

int main(int argc, char *argv[]) {
    printf("argc: %d \n", argc);
    for (int i = 0; i < argc; i++) {
        printf("argv %d: %s \n", i, argv[i]);
    }
    max_size = atoi(argv[1]);
    active_producer =argc-2;

    void *para = malloc(argc * sizeof(unsigned long) + sizeof(int) * 2);
    (*(int *) para) = argc - 2;
    (*((int *) para + 1)) = atoi(argv[1]);
    for (int i = 1; i < argc - 1; i++) {
        (*((char **) ((int *) para + 2) + i - 1)) = argv[i + 1];
    }
    thread_libinit(first_thread_start_function, para);
    return 0;
}


