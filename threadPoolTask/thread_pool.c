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
# define num_threads sysconf(_SC_NPROCESSORS_ONLN)
int _index =0;
// the next index to print.
pthread_mutex_t mutexQueue;
// mutex thread save the critical section
pthread_cond_t condQueue;
// cind thread for hold/wait and signal to threads for working
pthread_mutex_t mutextasks;
pthread_cond_t condtasks;

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
    while(task->id != _index){
        // the thread finish before the preview thread finish
        pthread_cond_wait(&condQueue,&mutexQueue);
    }
    printf("%s",task->message);
    fflush(stdout);
    _index++;
    taskCount--;
    free(task);
    pthread_cond_signal(&condtasks);
    // if there is a waiting thread it will be signal
}

void* Worker(void* argv){
    while(1){
        pthread_mutex_lock(&mutexQueue);
        while(taskCount == 0){
            pthread_cond_wait(&condQueue,&mutexQueue);
        }
        TextData* _node = _firstQ;
        _firstQ = _firstQ->next;
        executeTask(_node, (pflag)argv);
        pthread_mutex_unlock(&mutexQueue);
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
    int i;
    for(i = 0; i < num_threads; i++){
        pthread_create(&th[i], NULL, &Worker, _fl);
    }
    int k = atoi(argv[2]);
    i = 0;
    while(1)
    {
        TextData* node = malloc(sizeof(TextData));
        node->id = i;
        node->message = malloc(sizeof(char) * 1024);
        node->key_num = k;
        if (fgets(node->message, 1024, stdin) == NULL) {
            // EOF
            // pthread_cond_signal(&condQueue);
            break;
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
        pthread_cond_signal(&condQueue);
        while(taskCount == 10){
            pthread_cond_wait(&condtasks,&mutextasks);
        }
    }
    for(i = 0; i < num_threads; i++){
        pthread_join(th[i], NULL);
    }


    pthread_mutex_destroy(&mutexQueue);
    pthread_cond_destroy(&condQueue);


    return 0;
}