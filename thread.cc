//
// Created by 86152 on 2020/2/29.
//
#include <ucontext.h>
#include <queue>
#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <set>
#include "thread.h"
#include "interrupt.h"

using namespace std;

typedef struct {
    ucontext_t* ct;
    set<unsigned int>* locks;
}ct_with_locks_t;

typedef struct {
    ct_with_locks_t ct_with_locks;
    unsigned int lock_waiting;
} ct_with_locks_waiting_t;

typedef struct {
    ct_with_locks_t ct_with_locks;
    unsigned int lock_waiting;
    unsigned int cond_waiting;
} ct_with_locks_cv_t;

bool thread_libinited = false;
ucontext_t * ct_running;
set<unsigned int>* locks_running;
set<unsigned int>* locks_debug;
ucontext_t ct_safe;
char ct_safe_stack[STACK_SIZE];
queue<ct_with_locks_t> ct_queue_runnable;
vector<ct_with_locks_waiting_t> ct_queue_waiting;
vector<ct_with_locks_cv_t> ct_queue_cv;
set<unsigned int> locks_using;
const int mutex = 6666;

int deadlock_end(void) {
    printf("Thread library exiting.\n");
    interrupt_enable();
    return 0;
}

int thread_delete_switch(void) {

    delete(locks_running);
    free(ct_running->uc_stack.ss_sp);
    free(ct_running);

    if(ct_queue_runnable.empty()) {
        printf("Thread library exiting.\n");

        interrupt_enable();

        return 0;
    }

    ct_with_locks_t ct_with_locks_next = ct_queue_runnable.front();
    ct_queue_runnable.pop();
    ucontext_t * ct_next = ct_with_locks_next.ct;
    set<unsigned int>* locks_next = ct_with_locks_next.locks;
    locks_running = locks_next;
    ct_running = ct_next;
    if(setcontext(ct_next)) {
        interrupt_enable();
        return -1;
    }

    //interrupt_enable();

    return 0;
}


int thread_start(thread_startfunc_t func, void *arg) {

    interrupt_enable();

    func(arg);

    interrupt_disable();

    makecontext(&ct_safe, (void (*)()) thread_delete_switch, 0);
    if(setcontext(&ct_safe)) {
        interrupt_enable();
        return -1;
    }

    //interrupt_enable();

    return 0;
}


int thread_libinit(thread_startfunc_t func, void *arg) {


    if(thread_libinited) {
        return -1;
    }
    thread_libinited = true;

    interrupt_disable();

    if(getcontext(&ct_safe)) {
        interrupt_enable();
        return -1;
    }
    ct_safe.uc_stack.ss_sp = &ct_safe_stack;
    ct_safe.uc_stack.ss_size = STACK_SIZE;
    ct_safe.uc_stack.ss_flags = 0;
    ct_safe.uc_link = NULL;

    ucontext_t * ct_next = (ucontext_t *)malloc(sizeof(ucontext_t));
    if (! ct_next) {
        interrupt_enable();
        return -1;
    }
    if(getcontext(ct_next)) {
        free(ct_next);
        interrupt_enable();
        return -1;
    }
    ct_next->uc_stack.ss_sp = (char*)malloc(STACK_SIZE);
    if(! ct_next->uc_stack.ss_sp) {
        free(ct_next);
        interrupt_enable();
        return -1;
    }
    ct_next->uc_stack.ss_size = STACK_SIZE;
    ct_next->uc_stack.ss_flags = 0;
    ct_next->uc_link = NULL;
    makecontext(ct_next, (void (*)()) thread_start, 2, func, arg);
    locks_running = new set<unsigned int>();
    if(! locks_running) {
        free(ct_next->uc_stack.ss_sp);
        free(ct_next);
        interrupt_enable();
        return -1;
    }
    ct_running = ct_next;
    if(setcontext(ct_next)) {
        free(ct_next->uc_stack.ss_sp);
        free(ct_next);
        interrupt_enable();
        return -1;
    }

    //interrupt_enable();

    return 0;
}


int thread_create(thread_startfunc_t func, void *arg) {

    if(! thread_libinited) {
        return -1;
    }

    interrupt_disable();

    ucontext_t * ct_child = (ucontext_t *)malloc(sizeof(ucontext_t));
    if (! ct_child) {
        interrupt_enable();
        return -1;
    }
    if (getcontext(ct_child)) {
        free(ct_child);
        interrupt_enable();
        return -1;
    }
    ct_child->uc_stack.ss_sp = (char*)malloc(STACK_SIZE);
    if(! ct_child->uc_stack.ss_sp) {
        free(ct_child);
        interrupt_enable();
        return -1;
    }
    ct_child->uc_stack.ss_size = STACK_SIZE;
    ct_child->uc_stack.ss_flags = 0;
    ct_child->uc_link = NULL;
    makecontext(ct_child, (void (*)()) thread_start, 2, func, arg);

    set<unsigned int>* locks_child = new set<unsigned int>();

    ct_with_locks_t ct_with_locks_child;
    ct_with_locks_child.ct = ct_child;
    ct_with_locks_child.locks = locks_child;
    ct_queue_runnable.push(ct_with_locks_child);

    interrupt_enable();

    return 0;
}


int thread_yield(void) {
    if(! thread_libinited) {
        return -1;
    }

    interrupt_disable();

    if(ct_queue_runnable.empty()) {
        interrupt_enable();
        return 0;
    }

    ct_with_locks_t ct_with_locks_running;
    ct_with_locks_running.ct = ct_running;
    ct_with_locks_running.locks = locks_running;
    ct_queue_runnable.push(ct_with_locks_running);


    ct_with_locks_t ct_with_locks_next = ct_queue_runnable.front();
    ct_queue_runnable.pop();
    ucontext_t * ct_next = ct_with_locks_next.ct;
    set<unsigned int>* locks_next = ct_with_locks_next.locks;
    ucontext_t * ct_now = ct_running;
    locks_running = locks_next;
    ct_running = ct_next;
    if(swapcontext(ct_now, ct_next)) {
        interrupt_enable();
        return -1;
    }

    interrupt_enable();

    return 0;
}


int thread_lock(unsigned int lock) {


    if(! thread_libinited) {
        return -1;
    }

    interrupt_disable();

    if(locks_running->find(lock) != locks_running->end()) {
        interrupt_enable();
        return -1;
    }

    while(locks_using.find(lock) != locks_using.end() &&
    locks_running->find(lock) == locks_running->end()) {
        if (ct_queue_runnable.empty()) {
            makecontext(&ct_safe, (void (*)()) deadlock_end, 0);
            if (setcontext(&ct_safe)) {
                interrupt_enable();
                return -1;
            }
            interrupt_enable();
            return -1;
        }

        ct_with_locks_t ct_with_locks_running;
        ct_with_locks_running.ct = ct_running;
        ct_with_locks_running.locks = locks_running;
        ct_with_locks_waiting_t ct_with_locks_waiting_running;
        ct_with_locks_waiting_running.ct_with_locks = ct_with_locks_running;
        ct_with_locks_waiting_running.lock_waiting = lock;
        ct_queue_waiting.push_back(ct_with_locks_waiting_running);

        ct_with_locks_t ct_with_locks_next = ct_queue_runnable.front();
        ct_queue_runnable.pop();
        ucontext_t *ct_next = ct_with_locks_next.ct;
        set<unsigned int> *locks_next = ct_with_locks_next.locks;
        ucontext_t *ct_now = ct_running;

        locks_debug = locks_running;

        locks_running = locks_next;
        ct_running = ct_next;

        if (swapcontext(ct_now, ct_next)) {
            interrupt_enable();
            return -1;
        }
    }
    locks_running->insert(lock);
    locks_using.insert(lock);

    interrupt_enable();

    return 0;
}


int thread_unlock(unsigned int lock) {

    if(! thread_libinited) {
        return -1;
    }

    interrupt_disable();


    if(locks_running->find(lock) == locks_running->end()) {
        interrupt_enable();
        return -1;
    }

    locks_running->erase(lock);
    locks_using.erase(lock);


    std::vector<ct_with_locks_waiting_t>::iterator iter = ct_queue_waiting.begin();
    while(iter != ct_queue_waiting.end()) {
        if(iter->lock_waiting == lock) {
            locks_using.insert(lock);
            iter->ct_with_locks.locks->insert(lock);

            ct_queue_runnable.push(iter->ct_with_locks);
            ct_queue_waiting.erase(iter);

            break;
        } else {
            iter++;
        }
    }

    interrupt_enable();

    return 0;
}


int thread_lock_unatomic(unsigned int lock) {

    if(locks_running->find(lock) != locks_running->end()) {
        return -1;
    }

    while(locks_using.find(lock) != locks_using.end() &&
          locks_running->find(lock) == locks_running->end()) {
        if (ct_queue_runnable.empty()) {
            makecontext(&ct_safe, (void (*)()) deadlock_end, 0);
            if (setcontext(&ct_safe)) {
                return -1;
            }
            return -1;
        }

        ct_with_locks_t ct_with_locks_running;
        ct_with_locks_running.ct = ct_running;
        ct_with_locks_running.locks = locks_running;
        ct_with_locks_waiting_t ct_with_locks_waiting_running;
        ct_with_locks_waiting_running.ct_with_locks = ct_with_locks_running;
        ct_with_locks_waiting_running.lock_waiting = lock;
        ct_queue_waiting.push_back(ct_with_locks_waiting_running);

        ct_with_locks_t ct_with_locks_next = ct_queue_runnable.front();
        ct_queue_runnable.pop();
        ucontext_t *ct_next = ct_with_locks_next.ct;
        set<unsigned int> *locks_next = ct_with_locks_next.locks;
        ucontext_t *ct_now = ct_running;

        locks_debug = locks_running;

        locks_running = locks_next;
        ct_running = ct_next;

        if (swapcontext(ct_now, ct_next)) {
            return -1;
        }
    }
    locks_running->insert(lock);
    locks_using.insert(lock);

    return 0;
}


int thread_unlock_unatomic(unsigned int lock) {

    if(locks_running->find(lock) == locks_running->end()) {
        return -1;
    }

    locks_running->erase(lock);
    locks_using.erase(lock);


    std::vector<ct_with_locks_waiting_t>::iterator iter = ct_queue_waiting.begin();
    while(iter != ct_queue_waiting.end()) {
        if(iter->lock_waiting == lock) {
            locks_using.insert(lock);
            iter->ct_with_locks.locks->insert(lock);

            ct_queue_runnable.push(iter->ct_with_locks);
            ct_queue_waiting.erase(iter);
            break;
        } else {
            iter++;
        }
    }

    return 0;
}


int thread_wait(unsigned int lock, unsigned int cond) {

    if(! thread_libinited) {
        return -1;
    }

    interrupt_disable();

    if(thread_unlock_unatomic(lock)) {
        interrupt_enable();
        return -1;
    }

    if(ct_queue_runnable.empty()) {
        makecontext(&ct_safe, (void (*)()) deadlock_end, 0);
        if(setcontext(&ct_safe)) {
            interrupt_enable();
            return -1;
        }
        interrupt_enable();
        return -1;
    }

    ct_with_locks_t ct_with_locks_running;
    ct_with_locks_running.ct = ct_running;
    ct_with_locks_running.locks = locks_running;
    ct_with_locks_cv_t ct_with_locks_waiting_running;
    ct_with_locks_waiting_running.ct_with_locks = ct_with_locks_running;
    ct_with_locks_waiting_running.lock_waiting = lock;
    ct_with_locks_waiting_running.cond_waiting = cond;
    ct_queue_cv.push_back(ct_with_locks_waiting_running);

    ct_with_locks_t ct_with_locks_next = ct_queue_runnable.front();
    ct_queue_runnable.pop();
    ucontext_t * ct_next = ct_with_locks_next.ct;
    set<unsigned int>* locks_next = ct_with_locks_next.locks;
    ucontext_t * ct_now = ct_running;
    locks_running = locks_next;
    ct_running = ct_next;


    if(swapcontext(ct_now, ct_next)) {
        interrupt_enable();
        return -1;
    }

    if(thread_lock_unatomic(lock)) {
        interrupt_enable();
        return -1;
    }

    interrupt_enable();

    return 0;
}

int thread_signal(unsigned int lock, unsigned int cond) {

    if(! thread_libinited) {
        return -1;
    }

    interrupt_disable();

    for(std::vector<ct_with_locks_cv_t>::iterator it = ct_queue_cv.begin();
        it != ct_queue_cv.end(); ++it) {
        if(it->lock_waiting == lock && it->cond_waiting == cond) {
            ct_queue_runnable.push(it->ct_with_locks);
            ct_queue_cv.erase(it);
            break;
        }
    }

    interrupt_enable();

    return 0;

}

int thread_broadcast(unsigned int lock, unsigned int cond) {

    if(! thread_libinited) {
        return -1;
    }

    interrupt_disable();

    std::vector<ct_with_locks_cv_t>::iterator it = ct_queue_cv.begin();
    while(it != ct_queue_cv.end()) {
        if(it->lock_waiting == lock && it->cond_waiting == cond) {
            ct_queue_runnable.push(it->ct_with_locks);
            ct_queue_cv.erase(it);
        } else {
            it++;
        }
    }

    interrupt_enable();

    return 0;
}