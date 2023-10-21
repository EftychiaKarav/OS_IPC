#include "semoper.h"
#define ENC1_PATH_SIZE 255


int main(){

    int sem_id, segment_id;
    key_t sem_key, shm_key;
    pid_t pid;  //chlid process is ENC1
    int running = 1, dir_len;
    void *shared_memory = (void *)0;
    char buffer[TEXT_SIZE];   //temp buffer for entering messages
    char enc1_path[ENC1_PATH_SIZE];
    struct Shared_Data *p_shared_data, *ptr1;  //pointers for shared memory

    sem_key = get_key();
    shm_key = get_key();
    sem_id = semget(sem_key, 6, IPC_CREAT|0666);
    if(sem_id == -1){
        perror("semget() failed\n");
        exit(EXIT_FAILURE);
    }
    initialise_semaphores(sem_id);  //give every semaphore an initial value

    segment_id = shmget (shm_key, SHARED_MEMORY_SIZE, IPC_CREAT | 0666);
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
    p_shared_data->written_by_p1 = -1;
    ptr1->written_by_p1 = -1;

    getcwd(enc1_path, ENC1_PATH_SIZE);  //find the current working directory
    dir_len = strlen(enc1_path);
    strcpy(enc1_path + dir_len, "/enc1");  //the path for "enc1" executable file

    pid = fork();   //child process will execute enc1 process
    if (pid < 0){
            perror ("Error in Fork");
            exit(1);
        }
    if (pid == 0){      //enc1 process
        execl(enc1_path, "enc1", (char*)NULL);   
    }

    else{   //p1 process
        while (running){    //repeat until "TERM" message is sent
            Semaphore_DOWN(sem_id, 0);   //semaphore for p1 to execute its critical section
            if(p_shared_data->written_by_p1 == 0) {  //message sent by p2 user
                if (strncmp(p_shared_data->written_data, TERM, 4) == 0){
                    sleep(1);
                    printf("User P2 terminated the communication.\n");
                    break;
                }
                sleep(1);
                printf("P2 wrote: %s\n", p_shared_data->written_data);
            }
            printf("P1: Give a message: ");
            fgets(buffer, TEXT_SIZE, stdin);
            strcpy(p_shared_data->written_data, buffer);
            p_shared_data->written_by_p1 = 1;
            if (strncmp(buffer, TERM, 4) == 0) {
                running = 0;
            }
            Semaphore_UP(sem_id, 1);   //signal enc1 to start
        }
    }

    waitpid(pid, NULL, 0);    //wait for enc1 to finish if it hasn't already after "TERM" message
    Semaphore_DOWN(sem_id, 2);    //wait for p2 to finish first
    if (shmdt(shared_memory) == -1) {   //dettach shared memory segment
		fprintf(stderr, "shmdt failed\n");
		exit(EXIT_FAILURE);
	}
    shmctl(segment_id, IPC_RMID, 0);  //delete shared memory segment
    semctl(sem_id, 0, IPC_RMID, 0);  //delete all the semaphores
    sleep(1);
	exit(EXIT_SUCCESS);
}
