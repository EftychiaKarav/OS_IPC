#ifndef SEMOPER_H
#define SEMOPER_H                 /* Prevent accidental double inclusion */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/sem.h>

#define SHARED_MEMORY_SIZE 2*sizeof(struct Shared_Data)  //size for shared memory
#define TEXT_SIZE 512   //max length of message to be sent from p1 or p2
#define TERM "TERM"   //message for termination of communication


struct Shared_Data{
    int written_by_p1;   //0 if message is written by p2, 1 if message is written by p1
    char written_data[TEXT_SIZE];  //the message to be sent
};

key_t get_key();
void initialise_semaphores(int sem_id);
void Semaphore_UP(int sem_id, int index);
void Semaphore_DOWN(int sem_id, int index);

#if ! defined(__FreeBSD__) && ! defined(__OpenBSD__) && \
                ! defined(__sgi) && ! defined(__APPLE__)
                /* Some implementations already declare this union */

union semun {                   /* Used in calls to semctl() */
    int                 val;
    struct semid_ds *   buf;
    unsigned short *    array;
#if defined(__linux__)
    struct seminfo *    __buf;
#endif
};

#endif

#endif
