//
// Created by 杜悦 on 2020/9/10.
//
#include <ucontext.h>
#include <queue>
#include <vector>
#include <set>
#include "thread.h"
#include "interrupt.h"
#include<stdlib.h>
#include <stdio.h>

using namespace std;

bool isThreadInital = false;
ucontext_t * running_tcb;
ucontext_t ct_safe;
char ct_safe_stack[STACK_SIZE];
set<unsigned int>* running_locks;
set<unsigned int> locks_using;

typedef struct {
    ucontext_t* tcb;
    set<unsigned int>* locks;
}tcb_locks;

typedef struct {
    tcb_locks tcbLocks;
    unsigned int lock_waiting;
} tcb_locks_wait;

typedef struct {
    tcb_locks tcbLocks;
    unsigned int lock_waiting;
    unsigned int cv_waiting;
} tcb_locks_wait_cv;

queue<tcb_locks> readyQueue;
vector<tcb_locks_wait> waitQueue;
vector<tcb_locks_wait_cv> waitCvQueue;


int thread_delete(void) {
    interrupt_disable();
    delete (running_locks);
    free(running_tcb->uc_stack.ss_sp);
    free(running_tcb);

    if (readyQueue.empty()) {
        printf("Thread library exiting.\n");
        interrupt_enable();
        return 0;
    }

    tcb_locks readyThread = readyQueue.front();
    readyQueue.pop();
    ucontext_t *readyThreadTcb = readyThread.tcb;
    set<unsigned int> *readyThreadLocks = readyThread.locks;
    running_tcb = readyThreadTcb;
    setcontext(readyThreadTcb);
    return 0;
}

int thread_start(thread_startfunc_t func, void *arg){
    interrupt_enable();
    func(arg);
    interrupt_disable();

    makecontext(&ct_safe, (void (*)()) thread_delete, 0);
    setcontext(&ct_safe);

    interrupt_enable();
    return 0;
}


int thread_libinit(thread_startfunc_t func, void *arg){
    interrupt_disable();
    if (isThreadInital){
        interrupt_enable();
        return -1;
    }

    isThreadInital = true;

    ct_safe.uc_stack.ss_sp = &ct_safe_stack;
    ct_safe.uc_stack.ss_size = STACK_SIZE;
    ct_safe.uc_stack.ss_flags = 0;
    ct_safe.uc_link = NULL;

    ucontext_t * currentTcb = (ucontext_t *)malloc(sizeof(ucontext_t));
    if (! currentTcb) {
        interrupt_enable();
        return -1;
    }

    currentTcb->uc_stack.ss_size = STACK_SIZE;
    currentTcb->uc_stack.ss_flags = 0;
    currentTcb->uc_link = NULL;
    currentTcb->uc_stack.ss_sp = (char*)malloc(STACK_SIZE);

    if(! currentTcb->uc_stack.ss_sp) {
        free(currentTcb);
        interrupt_enable();
        return -1;
    }

    makecontext(currentTcb, (void (*)()) thread_start, 2, func, arg);
    running_locks = new set<unsigned int>();
    if(! running_locks) {
        free(currentTcb->uc_stack.ss_sp);
        free(currentTcb);
        interrupt_enable();
        return -1;
    }
    running_tcb = currentTcb;
    setcontext(currentTcb);

    interrupt_enable();
    return 0;
}

int thread_yield(void) {
    interrupt_disable();
    if(! isThreadInital) {
        interrupt_enable();
        return -1;
    }

    tcb_locks runningThread;
    runningThread.locks = running_locks;
    runningThread.tcb = running_tcb;
    readyQueue.push(runningThread);

    if(readyQueue.empty()){
        interrupt_enable();
        return 0;
    }

    tcb_locks readyThread = readyQueue.front();
    readyQueue.pop();
    ucontext_t  * readyThreadTcb = readyThread.tcb;
    set<unsigned int>* readyThreadLocks = readyThread.locks;

    ucontext_t  * currentTcb = running_tcb;
    running_tcb = readyThreadTcb;
    running_locks = readyThreadLocks;
    if(swapcontext(currentTcb, readyThreadTcb)) {
        interrupt_enable();
        return -1;
    }

    interrupt_enable();
    return 0;
}


int thread_create(thread_startfunc_t func, void *arg){
    interrupt_disable();
    if(! isThreadInital) {
        interrupt_enable();
        return -1;
    }
    ucontext_t * child_thread = (ucontext_t *)malloc(sizeof(ucontext_t));

    if (! child_thread) {
        interrupt_enable();
        return -1;
    }
    child_thread->uc_stack.ss_sp = (char*)malloc(STACK_SIZE);
    if(! child_thread->uc_stack.ss_sp) {
        free(child_thread);
        interrupt_enable();
        return -1;
    }
    child_thread->uc_stack.ss_size = STACK_SIZE;
    child_thread->uc_stack.ss_flags = 0;
    child_thread->uc_link = NULL;
    makecontext(child_thread, (void (*)()) thread_start, 2, func, arg);

    set<unsigned int>* locks_child = new set<unsigned int>();
    if(! locks_child) {
        free(child_thread->uc_stack.ss_sp);
        free(child_thread);
        interrupt_enable();
        return -1;
    }

    tcb_locks tcb_locks_child;
    tcb_locks_child.tcb = child_thread;
    tcb_locks_child.locks = locks_child;
    readyQueue.push(tcb_locks_child);

    interrupt_enable();

    return 0;
}


int thread_lock(unsigned int lock){
    interrupt_disable();
    if(! isThreadInital) {
        interrupt_enable();
        return -1;
    }

    if(running_locks->count(lock) !=0){
        interrupt_enable();
        return -1;
    }

    if(locks_using.count(lock) == 0){
        running_locks->insert(lock);
        locks_using.insert(lock);
        interrupt_enable();
        return 0;
    }
    else{
        if (readyQueue.empty()) {
            interrupt_enable();
            return -1;
        }
        else{
            tcb_locks runningThread;
            runningThread.locks = running_locks;
            runningThread.tcb = running_tcb;

            tcb_locks_wait runningThread_waitLock;
            runningThread_waitLock.tcbLocks = runningThread;
            runningThread_waitLock.lock_waiting = lock;
            waitQueue.push_back(runningThread_waitLock);

            tcb_locks readyThread = readyQueue.front();
            readyQueue.pop();
            ucontext_t  * readyThreadTcb = readyThread.tcb;
            set<unsigned int>* readyThreadLocks = readyThread.locks;

            ucontext_t  * currentTcb = running_tcb;
            running_tcb = readyThreadTcb;
            running_locks = readyThreadLocks;
            if(swapcontext(currentTcb, readyThreadTcb)) {
                interrupt_enable();
                return -1;
            }
        }
    }
    interrupt_enable();
    return 0;
}

int thread_unlock(unsigned int lock){
    interrupt_disable();
    if(! isThreadInital) {
        interrupt_enable();
        return -1;
    }
    if(running_locks->count(lock) == 0){
        interrupt_enable();
        return -1;
    }
    running_locks->erase(lock);
    locks_using.erase(lock);

    std::vector<tcb_locks_wait>::iterator iter = waitQueue .begin();
    while(iter != waitQueue.end()) {
        if(iter->lock_waiting == lock) {
            locks_using.insert(lock);
            iter->tcbLocks.locks->insert(lock);
            readyQueue.push(iter->tcbLocks);
            waitQueue.erase(iter);
            break;
        } else {
            iter++;
        }
    }

    interrupt_enable();
    return 0;
}

int thread_wait(unsigned int lock, unsigned int cond){
    interrupt_disable();
    if(! isThreadInital) {
        interrupt_enable();
        return -1;
    }

    if(thread_unlock(lock)) {
        interrupt_enable();
        return -1;
    }
    interrupt_disable();

    if (readyQueue.empty()) {
        interrupt_enable();
        return -1;
    }

    tcb_locks runningThread;
    runningThread.locks = running_locks;
    runningThread.tcb = running_tcb;

    tcb_locks_wait_cv runningThread_waitLockCv;
    runningThread_waitLockCv.tcbLocks = runningThread;
    runningThread_waitLockCv.lock_waiting = lock;
    runningThread_waitLockCv.cv_waiting = cond;
    waitCvQueue.push_back(runningThread_waitLockCv);

    tcb_locks readyThread = readyQueue.front();
    readyQueue.pop();
    ucontext_t  * readyThreadTcb = readyThread.tcb;
    set<unsigned int>* readyThreadLocks = readyThread.locks;

    ucontext_t  * currentTcb = running_tcb;
    running_tcb = readyThreadTcb;
    running_locks = readyThreadLocks;

    if(swapcontext(currentTcb, readyThreadTcb)) {
        interrupt_enable();
        return -1;
    }

    if(thread_lock(lock)) {
        interrupt_enable();
        return -1;
    }

    interrupt_enable();
    return 0;

}

int thread_signal(unsigned int lock, unsigned int cond){
    interrupt_disable();
    if(! isThreadInital) {
        interrupt_enable();
        return -1;
    }

    for(std::vector<tcb_locks_wait_cv>::iterator it = waitCvQueue.begin();
        it != waitCvQueue.end(); ++it) {
        if(it->lock_waiting == lock && it->cv_waiting == cond) {
            readyQueue.push(it->tcbLocks);
            waitCvQueue.erase(it);
            break;
        }
    }

    interrupt_enable();
    return 0;
}

int thread_broadcast(unsigned int lock, unsigned int cond){
    interrupt_disable();
    if(! isThreadInital) {
        interrupt_enable();
        return -1;
    }

    for(std::vector<tcb_locks_wait_cv>::iterator it = waitCvQueue.begin();
        it != waitCvQueue.end(); ++it) {
        if(it->lock_waiting == lock && it->cv_waiting == cond) {
            readyQueue.push(it->tcbLocks);
            waitCvQueue.erase(it);
            break;
        }
    }

    interrupt_enable();
    return 0;
}