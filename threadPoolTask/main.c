#include <stdio.h>
#include <pthread.h>

// gcc main.c -lpthread
void * runner1(void* argc){
    printf("hello you! \n");
    fflush(stdout);
}


int main(){
    int a;
    pthread_t pid1;
    pthread_create(&pid1, NULL, &runner1, NULL);
    pthread_join(pid1, NULL);
    scanf("%d", &a);
    printf("%d", a);
    fflush(stdout);
    scanf("%d", &a);
    printf("%d", a);
    return 0;
}