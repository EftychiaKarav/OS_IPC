// AM: 1115201800062
// KARAVANGELI EFTYCHIA

#include "semoper.h"
#define ENC2_PATH_SIZE 255


int main(){

    int sem_id, segment_id;
    key_t sem_key, shm_key;
    pid_t pid;   //child process is ENC2
    int running = 1, dir_len;
    void *shared_memory = (void *)0;
    char buffer[TEXT_SIZE];   //temp buffer for entering messages
    char enc2_path[ENC2_PATH_SIZE];
    struct Shared_Data *p_shared_data, *ptr1;  //pointers for shared memory

    sem_key = get_key();
    shm_key = get_key();
    sem_id = semget(sem_key, 0, 0);   //obtain the existing semaphores, created by p1 already
    
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
    p_shared_data->written_by_p1 = -1;
    ptr1->written_by_p1 = -1;

    getcwd(enc2_path, ENC2_PATH_SIZE);   //find the current working directory
    dir_len = strlen(enc2_path);
    strcpy(enc2_path + dir_len, "/enc2");  //the path for "enc2" executable file

    pid = fork();   //child process will execute enc2 process
    if (pid < 0){
            perror ("Error in Fork");
            exit(1);
        }
    if (pid == 0){  //enc2 process
        execl(enc2_path, "enc1", (char*)NULL);   
    }

    else{   //p2 process
        while (running){   //repeat until "TERM" message is sent
        //blocked until it is signaled by enc2
            Semaphore_DOWN(sem_id, 4); //semaphore for p2 to execute its critical section
            if(p_shared_data->written_by_p1 == 1) {  //message sent by p1 user
                if (strncmp(p_shared_data->written_data, TERM, 4) == 0){
                    sleep(1);
                    printf("User P1 terminated the communication.\n");
                    break;
                }
                sleep(1);
                printf("P1 wrote: %s\n", p_shared_data->written_data);

            }
            printf("P2: Give a message: ");
            fgets(buffer, TEXT_SIZE, stdin);
            strcpy(p_shared_data->written_data, buffer);
            p_shared_data->written_by_p1 = 0;
            if (strncmp(buffer, TERM, 4) == 0) {
                running = 0;
            }
            Semaphore_UP(sem_id, 1);   //signal enc1 to start             
        }
    }
    waitpid(pid, NULL, 0);   //wait for enc2 to finish if it hasn't already after "TERM" message

    if (shmdt(shared_memory) == -1) {   //dettach shared memory segment
		fprintf(stderr, "shmdt failed\n");
		exit(EXIT_FAILURE);
	}
    Semaphore_UP(sem_id, 2);  //signal p1 to delete shared memory and semaphores
    sleep(2);   //wait for a while
	exit(EXIT_SUCCESS);
}
