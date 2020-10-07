//
// Created by 86152 on 2020/3/8.
//

#include <stdio.h>
#include "thread.h"

void child0(void *a) {
    printf("%d ", thread_wait(100, 100));
    printf("%d ", thread_signal(100, 100));
    printf("%d ", thread_broadcast(100, 100));

    printf("%d ", thread_lock(100));

    printf("%d ", thread_signal(100, 100));
    printf("%d ", thread_signal(101, 100));
    printf("%d ", thread_broadcast(100, 100));
    printf("%d ", thread_broadcast(101, 100));
    printf("%d ", thread_wait(101, 100));
    printf("%d ", thread_wait(100, 200));

    printf("00 ");

    printf("%d ", thread_unlock(200));
    printf("%d ", thread_unlock(100));

    printf("01\n");
}

void child1(void *a) {
    printf("%d ", thread_lock(100));
    printf("%d ", thread_wait(100, 200));
    printf("10 ");
    printf("%d ", thread_unlock(200));
    printf("%d ", thread_unlock(100));
    printf("11\n");
}

void child2(void *a) {
    printf("%d ", thread_lock(100));
    printf("%d ", thread_wait(100, 200));
    printf("20 ");
    printf("%d ", thread_unlock(200));
    printf("%d ", thread_unlock(100));
    printf("21 ");
}

void child3(void *a) {
    printf("%d ", thread_lock(100));
    printf("30 ");
    printf("%d ", thread_wait(100, 4));
    printf("31 ");
    printf("%d\n", thread_unlock(100));
}

void child4(void *a) {
    printf("%d ", thread_lock(100));
    printf("40 ");
    printf("%d ", thread_wait(100, 4));
    printf("41 ");
    printf("%d ", thread_unlock(100));
}

void child5(void *a) {
    printf("%d ", thread_lock(100));
    printf("50 ");
    printf("%d ", thread_wait(100, 4));
    printf("51 ");
    printf("%d\n", thread_unlock(100));
}

void child6(void *a) {
    printf("%d ", thread_lock(101));
    printf("60 ");
    printf("%d ", thread_wait(100, 4));
    printf("%d ", thread_wait(101, 4));
    printf("61 ");
    printf("%d ", thread_unlock(101));
}

void child7(void* a) {
    printf("%d ", thread_lock(100));
    printf("%d ", thread_wait(100, 4));
    printf("%d ", thread_unlock(100));
    printf("-1\n");
}

void parent(void *a) {
    ///1
    printf("Test 1:\n");
    printf("%d ", thread_wait(100, 100));
    printf("%d ", thread_signal(100, 100));
    printf("%d\n", thread_broadcast(100, 100));
    ///2
    printf("Test 2:\n");
    thread_create((thread_startfunc_t) child0, (void *) 100);
    thread_yield();
    printf("%d ", thread_signal(101, 200));
    thread_yield();
    printf("%d ", thread_signal(100, 100));
    thread_yield();
    printf("%d ", thread_signal(100, 200));
    thread_yield();
    ///3
    printf("Test 3:\n");
    thread_create((thread_startfunc_t) child1, (void *) 100);
    thread_yield();
    printf("%d ", thread_broadcast(101, 200));
    thread_yield();
    printf("%d ", thread_broadcast(100, 100));
    thread_yield();
    printf("%d ", thread_broadcast(100, 200));
    thread_yield();
    ///4
    printf("Test 4:\n");
    thread_create((thread_startfunc_t) child2, (void *) 100);
    thread_yield();
    printf("%d ", thread_lock(100));
    printf("%d ", thread_signal(101, 200));
    printf("%d ", thread_unlock(100));
    thread_yield();
    printf("%d ", thread_lock(100));
    printf("%d ", thread_signal(100, 100));
    printf("%d ", thread_unlock(100));
    thread_yield();
    printf("%d ", thread_lock(100));
    printf("%d ", thread_signal(100, 200));
    printf("%d ", thread_unlock(100));
    thread_yield();
    printf("%d ", thread_lock(100));
    printf("%d ", thread_unlock(200));
    printf("%d\n", thread_unlock(100));
    ///5
    printf("Test 5:\n");
    thread_create((thread_startfunc_t) child2, (void *) 100);
    thread_yield();
    printf("%d ", thread_lock(100));
    printf("%d ", thread_broadcast(101, 200));
    printf("%d ", thread_unlock(100));
    thread_yield();
    printf("%d ", thread_lock(100));
    printf("%d ", thread_broadcast(100, 100));
    printf("%d ", thread_unlock(100));
    thread_yield();
    printf("%d ", thread_lock(100));
    printf("%d ", thread_broadcast(100, 200));
    printf("%d ", thread_unlock(100));
    thread_yield();
    printf("%d ", thread_lock(100));
    printf("%d ", thread_unlock(200));
    printf("%d\n", thread_unlock(100));
    ///6
    printf("Test 6:\n");
    thread_create((thread_startfunc_t) child3, (void *) 100);
    thread_create((thread_startfunc_t) child4, (void *) 100);
    thread_create((thread_startfunc_t) child5, (void *) 100);
    thread_yield();
    printf("%d ", thread_signal(100, 200));
    thread_yield();
    printf("%d ", thread_signal(200, 4));
    thread_yield();
    printf("%d ", thread_signal(100, 4));
    thread_yield();
    thread_yield();
    thread_yield();
    ///7
    printf("Test 7:\n");
    printf("%d ", thread_broadcast(100, 200));
    thread_yield();
    printf("%d ", thread_broadcast(200, 4));
    thread_yield();
    printf("%d ", thread_broadcast(100, 4));
    thread_yield();
    ///8
    printf("Test 8:\n");
    printf("%d ", thread_lock(100));
    thread_create((thread_startfunc_t) child6, (void *) 100);
    thread_yield();
    thread_lock(101);
    thread_yield();
    thread_unlock(101);
    thread_yield();
    printf("%d ", thread_broadcast(101, 4));
    thread_yield();
    printf("%d\n", thread_unlock(100));
    ///9
    printf("Test 9:\n");
    thread_create((thread_startfunc_t) child7, (void *) 100);
    printf("%d ", thread_lock(100));
    printf("%d ", thread_wait(100, 5));
    printf("%d ", thread_unlock(100));
    printf("-1\n");
}

int main() {
    ///0
    printf("Test 0:\n");
    printf("%d ", thread_wait(100, 100));
    printf("%d ", thread_signal(100, 100));
    printf("%d\n", thread_broadcast(100, 100));
    printf("%d ", thread_libinit((thread_startfunc_t) parent, (void *)100));
}
