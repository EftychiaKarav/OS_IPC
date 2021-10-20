// AM: 1115201800062
// KARAVANGELI EFTYCHIA

#include "semoper.h"


key_t get_key(){         //get the key for semaphore struct and shared memory segment 
  key_t key;
  key = ftok("./p1.c", 1); //it is the same for all processes
  return key;
}

void Semaphore_UP(int sem_id, int index){
/* procedure to perform a V operation on semaphore of given index */
   struct sembuf leave;  /* define operation on semaphore with given index */

   leave.sem_num = index;/* define operation on semaphore with given index */
   leave.sem_op  = 1;    /* add 1 to value for V operation */
   leave.sem_flg = SEM_UNDO;    

   if (semop (sem_id, &leave, 1) == -1)
     {  perror ("error in semaphore operation");
        exit (1);
     }
}

void Semaphore_DOWN(int sem_id, int index){
    /* procedure to perform a P operation on a semaphore of given index */
  struct sembuf enter;  

   enter.sem_num = index;/* define operation on semaphore with given index */
   enter.sem_op  = -1;   /* subtract 1 to value for P operation */
   enter.sem_flg = SEM_UNDO;    

   if (semop (sem_id, &enter, 1) == -1)
     {  perror ("error in semaphore operation");
        exit (1);
     }
}

void initialise_semaphores(int sem_id){
  /* procedure to initialize every semaphore to given value */
    
    union semun sem_union; 
    int index =0;
    sem_union.val = 1;
    for (index=0; index<6; index++){
      //0 is for p1, which is the first process which enters its critical section
        if (index == 0){   
            sem_union.val = 1;
        }
        else{
            sem_union.val = 0;
        }
        if (semctl (sem_id, index, SETVAL, sem_union.val) < 0){
            perror ("error in initializing first semaphore");
            exit (1);
        }
    }
}
