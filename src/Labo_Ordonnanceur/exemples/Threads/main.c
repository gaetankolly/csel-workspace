/**
 * Copyright 2015 University of Applied Sciences Western Switzerland / Fribourg
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Project:	HEIA-FR / Embedded Systems Laboratory
 *
 * Abstract: Multi-threading
 *
 * Purpose: This module implements a simple multi-threading demo progam
 *
 * Autĥor   Daniel Gachet / Jean-Roland Schuler
 * Date:    17.11.2015
 */

#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

// ----------------------------------------------------------------------------
// Macros and constants
// ----------------------------------------------------------------------------

#define ARRAY_OF(x) (sizeof(x) / sizeof(x[0]))

// ----------------------------------------------------------------------------
// Global variables
// ----------------------------------------------------------------------------

static pthread_t thread_id[3];
static bool is_running = true;

// ----------------------------------------------------------------------------
// Local threads
// ----------------------------------------int------------------------------------

void cleaner(void* p) { printf("%s: thread end\n", (char*)p); }

// ----------------------------------------------------------------------------

void* thread_1(void* arg)
{
    pthread_cleanup_push(&cleaner, arg);

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

    while (is_running) {
        printf(
            "%s pid=%d, tid=%ld\n", (char*)arg, getpid(), syscall(SYS_gettid));
        sleep(2);
    }

    pthread_cleanup_pop(1);
    pthread_exit((void*)1);
    return (void*)0;
}

// ----------------------------------------------------------------------------

void* thread_2(void* arg)
{
    pthread_cleanup_push(cleaner, arg);

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

    while (is_running) {
        printf(
            "%s pid=%d, tid=%ld\n", (char*)arg, getpid(), syscall(SYS_gettid));
        sleep(2);
    }

    pthread_cleanup_pop(0);
    pthread_exit(0);
    return (void*)2;
}

// ----------------------------------------------------------------------------

void* thread_3(void* arg)
{
    while (is_running) {
        printf(
            "%s pid=%d, tid=%ld\n", (char*)arg, getpid(), syscall(SYS_gettid));
        sleep(2);
    }
    return (void*)3;
}

// ----------------------------------------------------------------------------
// Local methods
// ----------------------------------------------------------------------------

static void catch_signal(int signal)
{
    printf("signal = %d\n", signal);
    pthread_cancel(thread_id[0]);
    is_running = 0;
}

// ----------------------------------------------------------------------------

static void install_catch_signal()
{
    struct sigaction act = {
        .sa_handler = catch_signal,
    };
    sigemptyset(&act.sa_mask);
    sigaction(SIGINT, &act, 0);
    sigaction(SIGTSTP, &act, 0);
    sigaction(SIGTERM, &act, 0);
    sigaction(SIGABRT, &act, 0);
}

// ----------------------------------------------------------------------------
// Main program
// ----------------------------------------------------------------------------

int main(void)
{
    install_catch_signal();

    printf("Create threads... (pid=%d)\n", getpid());
    pthread_create(&thread_id[0], NULL, thread_1, "thread_1");
    pthread_create(&thread_id[1], NULL, thread_2, "thread_2");
    pthread_create(&thread_id[2], NULL, thread_3, "thread_3");

    printf("Wait until threads finished...\n");
    printf("Type return to exit all threads\n");
    getchar();
    is_running = false;
    for (int i = ARRAY_OF(thread_id) - 1; i >= 0; i--) {
        void* return_msg;
        pthread_join(thread_id[i], &return_msg);
        printf("thread_%d terminated with %p\n", i + 1, return_msg);
    }

    printf("Program stops...\n");

    return 0;
}
