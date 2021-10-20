// AM: 1115201800062
// KARAVANGELI EFTYCHIA

#include <openssl/md5.h>
#include "semoper.h"
#define CHAN_PATH_SIZE 255


int main(){

    int sem_id, segment_id;
    key_t sem_key, shm_key;
    int running = 1;
    void *shared_memory = (void *)0;
    unsigned char* checksum_old, *checksum_new;
    unsigned char *copy_mess;     
    struct Shared_Data *p_shared_data, *ptr1;  //pointers for shared memory

    sem_key = get_key();
    shm_key = get_key();
    sem_id = semget(sem_key, 0, 0); //obtain the existing semaphores, created by p1 already

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

    //enc2 process
    while(running){  //repeat until "TERM" message is sent
        Semaphore_DOWN(sem_id, 3);  //semaphore for enc2 to execute its critical section
        char *temp = NULL;
        //allocate memory to copy the old checksum created by enc1
        checksum_old = malloc(strlen(ptr1->written_data)- strlen(p_shared_data->written_data));
        //allocate memory to copy the message from channel, it may be changed or not
        copy_mess = malloc(strlen(p_shared_data->written_data)+1);
        strncpy((char*)copy_mess, ptr1->written_data, strlen(p_shared_data->written_data));
        copy_mess[strlen(p_shared_data->written_data)] ='\0';
        temp = ptr1->written_data + strlen(p_shared_data->written_data); //points at the character where checksum starts
        strcpy((char*)checksum_old, temp);  //obtain previous checksum       
        checksum_new = MD5(copy_mess, strlen((char*)copy_mess)+1, NULL);  //find new checksum
        //compare two checksums
        if (memcmp((char*)checksum_old, (char*)checksum_new, strlen((char*)checksum_old)) == 0){
            //they are same 
            if (p_shared_data->written_by_p1){ 
                Semaphore_UP(sem_id, 4);  //signal p2 to start
            }
            else if(!p_shared_data->written_by_p1){
                Semaphore_UP(sem_id, 0);  //signal p1 to start
            }
        }
        else{   //they are different so enc1 will try to send the message again
            if(p_shared_data->written_by_p1)
                printf("Message couldn't be sent. P1 will try again in a while.\n");
            else if(!p_shared_data->written_by_p1)
                printf("Message couldn't be sent. P2 will try again in a while.\n");
            sleep(3);   //wait for a while
            Semaphore_UP(sem_id, 1);  //signal enc1 to start
        }
            free(checksum_old);
            free(copy_mess);

        if (strncmp(p_shared_data->written_data, TERM, 4) == 0) 
            break;
    }
    if (shmdt(shared_memory) == -1) {  //dettach shared memory segment
		fprintf(stderr, "shmdt failed\n");
		exit(EXIT_FAILURE);
	}
    exit(EXIT_SUCCESS);
}
