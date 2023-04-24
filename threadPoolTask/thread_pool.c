// export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:.
// gcc thread_pool.c -o thread_pool -lpthread libCodec.so
// ./thread_pool

#include "codec.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include<unistd.h>

int _index =0;

typedef struct TextData{
   int  id;
   char *message;
   int  key_num;
}TextData;

pthread_mutex_t mutexQueue;
pthread_cond_t condQueue;

TextData* taskQueue[1024];
int taskCount = 0;

void executeTask(TextData task){
    encrypt(task.message,task.key_num);
    while(task.id != _index){
        pthread_cond_wait(&condQueue,&mutexQueue);
    }
    printf("%s",task.message);
    fflush(stdout);
    _index++;
    taskCount--;
    pthread_cond_signal(&condQueue);
}
void decryptexecuteTask(TextData task){
    decrypt(task.message,task.key_num);
    while(task.id != _index){
        pthread_cond_wait(&condQueue,&mutexQueue);
    }

    printf("%s",task.message);
    fflush(stdout);
    _index++;
    taskCount--;
    pthread_cond_signal(&condQueue);
}
void* enWorker(void* argv){
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
void* deWorker(void* argv){
    while(1){
        pthread_mutex_lock(&mutexQueue);
        if(_index == 10){
            pthread_mutex_unlock(&mutexQueue);
            break;
        }
        while(taskCount == 0){
            pthread_cond_wait(&condQueue,&mutexQueue);
        }
        decryptexecuteTask((*taskQueue)[_index]);
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
        perror("not get flag error");
        exit(0);
    }
    for(int i=0; i<10; i++){
        taskQueue[i] = malloc(sizeof(TextData));
    }   
    pthread_t th[4];
    pthread_mutex_init(&mutexQueue, NULL);
    pthread_cond_init(&condQueue, NULL);
    int i;
    
    if(!strcmp(flag,  "encrypt"))
    {
        //printf("%s\n",flag);
        for(i = 0; i < 4; i++){
            pthread_create(&th[i], NULL, &enWorker, NULL);
        }
    }
    else
    {
        //printf("d %s\n",flag);
        for(i = 0; i < 4; i++){
            pthread_create(&th[i], NULL, &deWorker, NULL);
        }
    }
    
    int k = atoi(argv[2]);
    i = 0;
    while(1)
    {
        (*taskQueue)[i].id = i;
        (*taskQueue)[i].message = malloc(sizeof(char) * 1024);
        (*taskQueue)[i].key_num = k;
        if (fgets((*taskQueue)[i].message, 1024, stdin) == NULL) {
            // EOF
            pthread_cond_signal(&condQueue);
            break;
        }
        
        //scanf("%s",(*taskQueue)[i].message);
        taskCount++;
        i++;
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