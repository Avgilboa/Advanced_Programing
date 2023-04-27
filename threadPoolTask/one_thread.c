#include "codec.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include<unistd.h>
#include <unistd.h>
#include <sys/time.h>

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
    char massage[1024];
    int k = atoi(argv[2]);
    int i = 0;
    while(1){
        if(fgets(massage, 1024, stdin) == NULL){
            continue;
        }
        if(!strcmp(flag, "encrypt"))
            encrypt(massage,k);
        else if(!strcmp(flag, "decrypt"))
            decrypt(massage,k);
        printf("%s", massage);
        fflush(stdout);
    }
    return 0;
}