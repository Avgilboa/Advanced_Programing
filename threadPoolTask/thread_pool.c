// export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:.
// gcc thread_pool.c -o thread_pool -lpthread libCodec.so
// ./thread_pool

#include "codec.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
int _index =0;

typedef struct TextData{
   int  id;
   char *message;
}TextData;

pthread_mutex_t mutexQueue;
pthread_cond_t condQueue;

TextData* taskQueue[10];
int taskCount = 0;

void executeTask(TextData task){
    encrypt(task.message,12);
    while(task.id != _index){
        pthread_cond_wait(&condQueue,&mutexQueue);
    }

    printf("encripted data: %s\n",task.message);
    _index++;
    taskCount--;
    pthread_cond_signal(&condQueue);
}
void* worker(void* argv){
    while(1){
        pthread_mutex_lock(&mutexQueue);
        if(_index == 10){
            pthread_mutex_unlock(&mutexQueue);
            break;
        }
        while(taskCount == 0){
            pthread_cond_wait(&condQueue,&mutexQueue);
        }
        executeTask((*taskQueue)[_index]);
        pthread_mutex_unlock(&mutexQueue);
    }
}

int main(){
    for(int i=0; i<10; i++){
        taskQueue[i] = malloc(sizeof(TextData));
    }   
    pthread_t th[4];
    pthread_mutex_init(&mutexQueue, NULL);
    pthread_cond_init(&condQueue, NULL);
    int i;


    for(i = 0; i < 4; i++){
        pthread_create(&th[i], NULL, &worker, NULL);
    }

    for(i=0 ; i<10 ; i++){
        (*taskQueue)[i].id = i;
        (*taskQueue)[i].message = malloc(1024);
        scanf("%s",(*taskQueue)[i].message);
        taskCount++;
        pthread_cond_signal(&condQueue);
    }





    for(i = 0; i < 4; i++){
        pthread_join(th[i], NULL);
    }

    for (i = 0; i < 10; i++)
    {
        free(taskQueue[i]->message);
        free(taskQueue[i]);
    }

    pthread_mutex_destroy(&mutexQueue);
    pthread_cond_destroy(&condQueue);


    return 0;
}