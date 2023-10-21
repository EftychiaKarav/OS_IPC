#include "semoper.h"
#include <openssl/md5.h>
#define CHAN_PATH_SIZE 255


int main(){

    int sem_id, segment_id;
    key_t sem_key, shm_key;
    pid_t pid;  //child process is channel
    int running = 1, dir_len;
    void *shared_memory = (void *)0;
    unsigned char* copy_mess = NULL;     
    unsigned char* checksum_old = NULL;
    char chan_path[CHAN_PATH_SIZE];
    struct Shared_Data *p_shared_data, *ptr1; 

    sem_key = get_key();
    shm_key = get_key();
    sem_id = semget(sem_key, 0, 0); //obtain the existing semaphores, created by p1 already
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

    getcwd(chan_path, CHAN_PATH_SIZE);  //find the current working directory
    dir_len = strlen(chan_path);
    strcpy(chan_path + dir_len, "/chan"); //the path for "chan" executable file

    pid = fork();  //child process will execute enc1 process
    if (pid < 0){
        perror ("Error in Fork");
        exit(1);
    }
    if (pid == 0){   //channel process
        execl(chan_path, "chan", (char*)NULL);  
    }

    else{   //enc1 process
        while (running){   //repeat until "TERM" message is sent
            Semaphore_DOWN(sem_id, 1);  //semaphore for enc1 to execute its critical section
            checksum_old = NULL;
            ptr1->written_data[0] = '\0';
            //allocate memory to copy the message sent to find its checksum
            copy_mess = malloc(strlen(p_shared_data->written_data)+1); 
            strcpy((char*)copy_mess, p_shared_data->written_data);
            checksum_old = MD5(copy_mess, strlen((char*)copy_mess)+1, NULL); //find checksum
            //copy initial message + checksum to shared memory part pointed by ptr1
            strcpy(ptr1->written_data, p_shared_data->written_data);
            strcat(ptr1->written_data, (char*)checksum_old);
            //apply '\0' character at last position
            ptr1->written_data[strlen(ptr1->written_data)] = '\0';
            free(copy_mess);
            Semaphore_UP(sem_id, 5);   //signal channel to start
            if (strncmp(p_shared_data->written_data, TERM, 4) == 0)
                break;
        }
    }

    waitpid(pid, NULL, 0);  //wait for chan to finish if it hasn't already after "TERM" message
    if (shmdt(shared_memory) == -1) {  //dettach shared memory segment
		fprintf(stderr, "shmdt failed\n");
		exit(EXIT_FAILURE);
	}
	exit(EXIT_SUCCESS);
}
