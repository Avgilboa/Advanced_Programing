// export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:.
// gcc thread_pool.c -o thread_pool -lpthread libCodec.so
// ./thread_pool

#include "codec.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include<unistd.h>
#include <unistd.h>
#include <sys/time.h>
# define num_threads sysconf(_SC_NPROCESSORS_ONLN)
int _index =0;
// the next index to print.
pthread_mutex_t mutexQueue;
// mutex thread save the critical section
pthread_cond_t condQueue;
// cind thread for hold/wait and signal to threads for working
pthread_mutex_t mutextasks;
pthread_cond_t condtasks;

pthread_mutex_t mutexadd;
pthread_cond_t condadd;


typedef struct TextData{
   int  id;
   char *message;
   int  key_num;
   struct TextData* next;
}TextData;
typedef struct flag{
    char flag[1024];
}flag, *pflag;
TextData* _firstQ = NULL ;
TextData* _lastQ = NULL;
// Queue for any line
int taskCount = 0;
// count the number of tasks in queue 

void executeTask(TextData* task, pflag flag){
    if(!strcmp(flag->flag, "encrypt"))
        encrypt(task->message,task->key_num);
    else if(!strcmp(flag->flag, "decrypt"))
        decrypt(task->message,task->key_num);
    pthread_mutex_lock(&mutexQueue);
    while(task->id != _index){
        //printf("The number of ID is %d\n", task->id);
        // the thread finish before the preview thread finish
        pthread_cond_wait(&condQueue,&mutexQueue);
    }
    //printf("THe ID that excute is: %d\n", task->id);
    printf("%s",task->message);
    fflush(stdout);
    _index++;
    free(task);
    pthread_mutex_unlock(&mutexQueue);
    pthread_cond_broadcast(&condQueue);
    // pthread_cond_signal(&condQueue);
    
    // if there is a waiting thread it will be signal
}

void* Worker(void* argv){
    while(1){
        // means that i excute all the relevant task that was in the queue
        while(taskCount == 0){
            pthread_cond_wait(&condtasks,&mutextasks);
        }
        TextData* _node = _firstQ;
        _firstQ = _firstQ->next;
        taskCount--;
        //pthread_cond_signal(&condadd);
        executeTask(_node, (pflag)argv);
        pthread_cond_broadcast(&condtasks);
        //printf("The number of task is: %d\n", taskCount);
    }
}


int main(int argc, char *argv[]){
    char *flag;
    if(!strcmp(argv[1],  "-d"))
    {
        flag = "decrypt";
    }
    else if(!strcmp(argv[1],  "-e"))
    {
        flag = "encrypt";
    }
    else{
        perror("Usage: ./thread_pool -e/-d key_num");
        exit(0);
    } 
    pthread_t th[num_threads];
    pflag _fl = malloc(sizeof(flag));
    strcpy(_fl->flag, flag);
    pthread_mutex_init(&mutexQueue, NULL);
    pthread_cond_init(&condQueue, NULL);
    pthread_mutex_init(&mutextasks, NULL);
    pthread_cond_init(&condtasks, NULL);
    pthread_mutex_init(&mutexadd, NULL);
    pthread_cond_init(&condadd, NULL);
    time_t start, end;
    start = time(NULL);
    int i;
    for(i = 0; i < num_threads; i++){
        pthread_create(&th[i], NULL, &Worker, _fl);
    }
    int k = atoi(argv[2]);
    i = 0;
    int stop =1;
    while(stop)
    {
        TextData* node = malloc(sizeof(TextData));
        node->id = i;
        node->message = malloc(sizeof(char) * 1024);
        node->key_num = k;
        if (fgets(node->message, 1024, stdin) == NULL) {
            // EOF
            //pthread_cond_signal(&condQueue);
            pthread_cond_broadcast(&condtasks);
            /*
            if(taskCount == 0){
                stop = 0;
                continue;
            }
        */
        }
        if( _firstQ == NULL) {
            _firstQ = node;
            _firstQ->next = _lastQ;
        }
        else{
            if(_lastQ == NULL){
                _lastQ = node;
                _firstQ->next = _lastQ;
            }
            else{
                _lastQ->next = node;
                _lastQ = _lastQ->next;
            }
        }
        taskCount++;
        i++;
        pthread_cond_broadcast(&condtasks);
        while (taskCount > num_threads)
        {
            //pthread_cond_wait(&condadd, &mutexadd);
        }
        
    }
    for(i = 0; i < num_threads; i++){
        pthread_join(th[i], NULL);
    }



    pthread_mutex_destroy(&mutexQueue);
    pthread_cond_destroy(&condQueue);
    pthread_mutex_destroy(&mutextasks);
    pthread_cond_destroy(&condtasks);
    pthread_mutex_destroy(&mutexadd);
    pthread_cond_destroy(&condadd);

    end = time(NULL);
    printf("Time taken: %.2fs\n", (double)(end - start));
    

    return 0;
}
