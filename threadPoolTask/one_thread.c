#include <stdio.h>
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
    time_t start, end;
    start = time(NULL);
    while(fgets(massage, 1024, stdin) != NULL){
        if(!strcmp(flag, "encrypt"))
            encrypt(massage,k);
        else if(!strcmp(flag, "decrypt"))
            decrypt(massage,k);
        printf("%s", massage);
    }
    end = time(NULL);
    printf("time: %ld\n", end - start);
    return 0;
}