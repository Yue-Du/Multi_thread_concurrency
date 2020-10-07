//
// Created by 86152 on 2020/3/6.
//

#include <stdio.h>
#include "thread.h"
#include <iostream>
#include <cstdlib>
#include <assert.h>
using namespace std;

int sum = 0;

void none(void* a) {
    return;
}

void grandchild0(void * a) {
    printf("%d ", sum);
    sum++;
    printf("%d ", *(int*) a);
}

void child0(void * a) {
    ///7
    printf("Test 7:\n");
    printf("%d \n", sum);
    sum++;
    ///8
    printf("Test 8:\n");
    printf("%d \n", *(int*) a);
    ///9
    printf("Test 9:\n");
    printf("%d \n", thread_yield());
    ///12
    printf("Test 12:\n");
}

void child1(void * a) {
    ///10
    printf("%d ", thread_create((thread_startfunc_t) grandchild0, (void *) 103));
    printf("%d\n", thread_create((thread_startfunc_t) grandchild0, (void *) 104));
}

void child2(void * a) {
    ///14
    printf("Test 14:\n");
    printf("%d ", sum);
    sum++;
    printf("%d \n", *(int*) a);
}

void parent(void* a) {
    ///3
    printf("Test 3:\n");
    printf("%d \n", *(int*)a);
    ///4
    printf("Test 4:\n");
    printf("%d ", thread_yield());
    printf("%d ", thread_yield());
    printf("%d ", thread_libinit((thread_startfunc_t) none, (void *)0));
    printf("\n");
    ///5
    printf("Test 5:\n");
    printf("%d \n", sum);
    sum++;
    ///6
    printf("Test 6:\n");
    printf("%d ", thread_create((thread_startfunc_t) child0, (void *) 101));
    printf("%d ", thread_create((thread_startfunc_t) child1, (void *) 102));
    printf("%d \n", thread_yield());
    ///11
    printf("Test 11:\n");
    printf("%d \n", thread_yield());
    ///13
    printf("Test 13:\n");
    printf("%d\n", thread_create((thread_startfunc_t) child2, (void *) 105));

}

int main() {
    ///0
    cout << "thread_create\n";
    printf("Test 0:\n");
    printf("%d \n", sum);
    sum++;
//    ///1
//    printf("Test 1:\n");
//    printf("%d ", thread_create((thread_startfunc_t) none, (void *) 0));
//    printf("%d ", thread_yield());
//    printf("\n");
//    ///2
//    printf("Test 2\n");
//    printf("%d ", thread_libinit((thread_startfunc_t) parent, (void *)100));
//    printf("-1 ");
//    printf("\n");
    return 0;
}