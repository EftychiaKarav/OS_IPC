#include "semoper.h"
#include <time.h>
#define PROBABILITY_OF_CHANGE 0.2


int get_probability(){
    double number;
    srand(time(NULL));
    number = (double)rand() / RAND_MAX;
    if (number <= PROBABILITY_OF_CHANGE)
        return 1;    //message will change
    return 0;    //message will not change
}

void change_message(char* copy_message, int length){
    int position, i;
    char new_char;
    for(i=0; i<5; i++){
        position = rand()%(length-1);   //position of '\n' which is the last character not to be changed
        new_char = (rand()%93) + 33;    //new characters have ascii values between 33 and 125
        copy_message[position] = new_char;
    }
}

int main(){

    int sem_id, segment_id, running = 1;
    key_t sem_key, shm_key;
    int change = -1;
    void *shared_memory = (void *)0;
    struct Shared_Data *p_shared_data, *ptr1;

    sem_key = get_key();
    shm_key = get_key();
    sem_id = semget(sem_key, 0, 0);  //obtain the existing semaphores, created by p1 already
    if(sem_id < 0){
        perror("semget() failed\n");
        exit(EXIT_FAILURE);
    }

    segment_id = shmget (shm_key, SHARED_MEMORY_SIZE, 0666); //obtain the existing shared memory segment, created by p1
    if (segment_id == -1) {
		fprintf(stderr, "shmget failed\n");
		exit(EXIT_FAILURE);
	}
    shared_memory = shmat(segment_id, (void*)0, 0);
    if (shared_memory == (void *)-1) {
		fprintf(stderr, "shmat failed\n");
		exit(EXIT_FAILURE);
	}
    //pointers which point to different parts of the shared memory segment 
    p_shared_data = (struct Shared_Data*)shared_memory;
    ptr1 = (struct Shared_Data*)shared_memory + 1;

    while (running){  //repeat until "TERM" message is sent
        Semaphore_DOWN(sem_id, 5);  //semaphore for channel to execute its critical section
        change = get_probability(); //0 if message doesn't change, 1 else
        if (strncmp(p_shared_data->written_data, TERM, 4) == 0){ 
            change = 0;  //message term doesn't change
        }
        if (change){
            change_message(ptr1->written_data, strlen(p_shared_data->written_data));
        } 
        Semaphore_UP(sem_id, 3);  //signal enc2 to start

        if (strncmp(p_shared_data->written_data, TERM, 4) == 0) 
            break;
    }
    if (shmdt(shared_memory) == -1) {   //dettach shared memory segment
		fprintf(stderr, "shmdt failed\n");
		exit(EXIT_FAILURE);
	}
	exit(EXIT_SUCCESS);
}
