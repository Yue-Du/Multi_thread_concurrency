//
// Created by 86152 on 2020/3/7.
//

#include <stdio.h>
#include "thread.h"



void child0(void *a) {
    printf("%d ", thread_unlock(100));
}

void child1(void *a) {
    printf("%d ", thread_lock(100));
    printf("%d ", thread_lock(101));
    thread_yield();
    printf("%d ", thread_unlock(100));
    printf("%d\n", thread_unlock(101));
}

void child2(void *a) {
    printf("%d ", thread_lock(4));
}

void child3(void *a) {
    printf("%d ", thread_unlock(4));
    printf("%d ", thread_lock(4));
    printf("-2\n");
}

void grandchild0(void *a) {
    printf("%d ", thread_unlock(100));
    printf("%d ", thread_lock(101));
    thread_yield();
    printf("%d ", thread_unlock(101));
    printf("%d ", thread_lock(44));
}

void child4(void *a) {
    printf("%d ", thread_lock(100));
    thread_create((thread_startfunc_t) grandchild0, (void *) 100);
    thread_yield();
    printf("%d ", thread_unlock(101));
    printf("%d ", thread_unlock(100));
    thread_yield();
    printf("%d ", thread_unlock(44));
}

void child5(void *a) {
    printf("%d ", thread_lock(101));
    printf("%d ", thread_lock(102));
    printf("%d ", thread_lock(103));

    printf("%d ", thread_lock(100));

    printf("%d ", thread_lock(101));
    printf("%d ", thread_lock(102));
    printf("%d ", thread_lock(103));

    printf("%d ", thread_unlock(101));
    printf("%d ", thread_unlock(102));
    printf("%d ", thread_unlock(103));

    printf("%d\n", thread_unlock(100));
}

void child6(void *a) {
    printf("%d ", thread_lock(100));
    thread_yield();
    printf("%d ", thread_lock(200));
    thread_yield();
    printf("16 ");
    printf("%d ", thread_unlock(200));
    thread_yield();
    printf("%d ", thread_lock(201));
    printf("17 ");
    printf("%d ", thread_unlock(201));
    printf("%d ", thread_unlock(100));
}

void child7(void *a) {
    printf("%d ", thread_lock(101));
    thread_yield();
    printf("%d ", thread_lock(200));
    printf("26 ");
    printf("%d ", thread_unlock(200));
    printf("%d ", thread_lock(201));
    thread_yield();
    printf("27 ");
    printf("%d ", thread_unlock(201));
    printf("%d ", thread_unlock(101));
}

void child8(void *a) {
    printf("8 ");
    printf("%d ", thread_lock(100));
    printf("8 ");
    thread_yield();
    printf("8 ");
    printf("%d ", thread_unlock(100));
    printf("8 ");
}

void child9(void * a) {
    printf("9 ");
    printf("%d ", thread_lock(100));
    printf("9 ");
    printf("%d ", thread_unlock(100));
    printf("9 ");
}

void child10(void * a) {
    printf("10 ");
    printf("%d ", thread_lock(100));
    printf("10 ");
    printf("%d ", thread_unlock(100));
    printf("10 ");
}

void child11(void *a) {
    printf("%d ", thread_lock(101));
    printf("%d ", thread_lock(100));
    printf("-1\n");
}

void parent(void* a) {
    ///1
    printf("Test 1:\n");
    printf("%d\n", thread_unlock(0));
    ///2
    printf("Test 2:\n");
    printf("%d ", thread_lock(100));
    printf("%d ", thread_lock(100));
    printf("%d ", thread_unlock(100));
    printf("%d\n", thread_unlock(100));
    ///3
    printf("Test 3:\n");
    printf("%d ", thread_lock(100));
    printf("%d ", thread_lock(101));
    printf("%d ", thread_unlock(100));
    printf("%d\n", thread_unlock(101));
    ///4
    printf("Test 4:\n");
    printf("%d ", thread_lock(100));
    printf("%d ", thread_lock(101));
    printf("%d ", thread_unlock(101));
    printf("%d\n", thread_unlock(100));
    ///5
    printf("Test 5:\n");
    printf("%d ", thread_lock(100));
    printf("%d ", thread_lock(101));
    thread_create((thread_startfunc_t) child0, (void *) 100);
    thread_yield();
    printf("%d ", thread_unlock(100));
    printf("%d\n", thread_unlock(101));
    ///6
    printf("Test 6:\n");
    thread_create((thread_startfunc_t) child1, (void *) 100);
    thread_yield();
    printf("%d ", thread_unlock(100));
    thread_yield();
    ///7
    printf("Test 7:\n");
    thread_create((thread_startfunc_t) child2, (void *) 100);
    thread_create((thread_startfunc_t) child3, (void *) 100);
    thread_yield();
    printf("%d\n", thread_unlock(4));
    ///8
    printf("Test 8:\n");
    thread_create((thread_startfunc_t) child4, (void *) 100);
    thread_yield();
    thread_yield();
    thread_yield();
    printf("%d\n", thread_unlock(44));
    ///9
    printf("Test 9:\n");
    printf("%d ", thread_lock(100));
    thread_create((thread_startfunc_t) child5, (void *) 100);
    thread_yield();
    printf("%d ", thread_unlock(100));
    thread_yield();
    ///10
    printf("Test 10:\n");
    thread_create((thread_startfunc_t) child6, (void *) 100);
    thread_create((thread_startfunc_t) child7, (void *) 100);
    thread_yield();
    printf("0 ");
    printf("%d ", thread_lock(100));
    printf("0 ");
    printf("%d ", thread_lock(101));
    printf("0 ");
    printf("%d ", thread_unlock(100));
    printf("0 ");
    printf("%d ", thread_unlock(101));
    printf("0\n");
    ///11
    printf("Test 11:\n");
    thread_create((thread_startfunc_t) child8, (void *) 100);
    thread_create((thread_startfunc_t) child9, (void *) 100);
    thread_create((thread_startfunc_t) child10, (void *) 100);
    thread_yield();
    printf("0 ");
    thread_lock(100);
    printf("0 ");
    thread_unlock(100);
    printf("0\n");
    ///12
    printf("Test 12:\n");
    printf("%d ", thread_lock(100));
    thread_create((thread_startfunc_t) child11, (void *) 100);
    thread_yield();
    printf("%d ", thread_lock(101));
    printf("-1\n");

}

int main() {
    ///0
    printf("Test 0:\n");
    printf("%d ", thread_lock(0));
    printf("%d\n", thread_unlock(0));
    thread_libinit((thread_startfunc_t) parent, (void *)100);

}