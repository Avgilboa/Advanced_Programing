// for the map we use this method: https://stackoverflow.com/questions/647054/porting-stdmap-to-c

#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "stdio.h"
#include "errno.h"
#include "stdlib.h"
#include "unistd.h"
#include <string.h>
#include<unistd.h>
#include <signal.h>
#include <search.h>
#include <stdlib.h>
#include <stdio.h>
#define SIGINT 2
typedef struct map{
    char key [1024];
    char value [1024];
}map;

/*include handler for cntrl c */
void sig_hand(int dummy){
    printf("\nyou pressed ctrl-c!\n");
    return;
}



int size_of_map = 0;
char history[20][1024] = {""};
char command[1024];
char *token;
int i, history_index = 0;
char *outfile;
char* error_outfile;
char promt[1024] = "hello";
int fd, amper, cp_redirect ,redirect, error_redirect, piping, retid, status, argc1,count_pipe;
int fildes[2];
int flag =1;
char *argv1[10], *argv2[10];
int counter;
int exevp_flag = 0;
int add_varb = 0;
char then_command[1024];
char else_command[1024];

// The handler of the cntrl + c 
void exe_c(map*,char **,int);
void pipe_command(map*,char*);
int main() {
signal(SIGINT, sig_hand);
map* root = (map*)malloc(1024 * sizeof(map));
while (1)
{
    exevp_flag = 0;
    if (flag){
        printf("%s: ", promt);
        fgets(command, 1024, stdin);
        command[strlen(command) - 1] = '\0';
    }
    flag =1;
    strcpy(history[history_index] , command);
    history_index = ( history_index + 1 ) %20;
    piping = 0;
    count_pipe = 0;
    /* parse command line */
    i = 0;
    char temp[1024];
    strcpy(temp, command);
    token = strtok (command," ");
    while (token != NULL)
    {
        argv1[i] = token;
        token = strtok (NULL, " ");
        i++;
        if (token && ! strcmp(token, "|")) {
            piping = 1;
            break;
        }
    }
    argv1[i] = NULL;
    argc1 = i;
    /* Is command empty */
    if (argv1[0] == NULL)
        continue;

    /* Does command contain pipe */
    if (piping) {
        pipe_command(root,temp);
    }
    else{
        exe_c(root,argv1,argc1);
    }
    piping = 0;
}
}
void exe_c(map* root,char **argv, int argc)
{
    if (argc == 1 && command[0] == '\033') {
        int place = 0;
        int counter_arrow = 0;
        --history_index;
        while (command[place] == '\033'){
            if(command[place + 1] == '[' && command[place + 2] == 'A'){
                counter_arrow++;
            }
            else if (command[place + 1] == '[' && command[place + 2] == 'B')
            {
                counter_arrow--;
            }
            place+=3;
        }
        if(counter_arrow < 0){
            return;
        }
        else{
            strcpy(command, history[(history_index - counter_arrow) % 20]);
            flag =0;
            return;
        }

    }
    
    if( !strcmp(argv[0], "if")){
        char fi_command[1024];
        scanf("%s", then_command);
        if(!strcmp(then_command, "then")){
            read(0, then_command, 1024);
            then_command[strlen(then_command) - 1] = '\0';
        }
        scanf("%s", else_command);
        if(!strcmp(else_command, "else")){
            read(0, else_command, 1024);
            else_command[strlen(else_command) - 1] = '\0';
        }
        scanf("%s", fi_command);
        if(strcmp(fi_command, "fi")){return;}
        char *temp_argv[10];
        for(int i=0;i<argc-1;i++){
            strcpy(temp_argv[i],argv[i+1]);
        }
        //temp_argv[argc-1] = NULL;
        exe_c(root, temp_argv, argc-1);
        if(status == 0){
            strcpy(command, then_command);
            flag =0;
            return;
        }
        else{
            strcpy(command, else_command);
            flag =0;
            return;
        }
        exit(1);
    }

    if(argc == 1 && !strcmp(argv[0], "!!")){
        strcpy(command, history[(history_index - 2) % 20]);
        printf("%s\n",history[(history_index - 2)% 20]);
        history_index = (history_index - 1)% 20;
        //strcpy(history[history_index] , command);
        flag =0;
        return;
    }

    if (argc == 3 && ! strcmp(argv[0], "prompt") && ! strcmp(argv[1], "=")) {
        strcpy(promt, argv[2]);
        return;
    }
    /* Does command line end with & */ 
    if (! strcmp(argv[argc1 - 1], "&")) {
        amper = 1;
        argv[argc - 1] = NULL;
        }
    else 
        amper = 0; 
    
    /* errors redirecting */
    if (argc > 1 && ! strcmp(argv[argc - 2], "2>")) {
        error_redirect = 1;
        argv[argc - 2] = NULL;
        error_outfile = argv[argc - 1];
        argc = argc - 2;
        }
    else 
        error_redirect = 0;

    /* redirection of IO*/
    if (argc > 1 && ! strcmp(argv[argc - 2], ">")) {
        redirect = 1;
        argv[argc - 2] = NULL;
        outfile = argv[argc - 1];
        }
    else 
        redirect = 0; 

    /* copy the IO to the end of exist file */
    if(!redirect && argc > 1 && ! strcmp(argv[argc -2 ], ">>")){
        cp_redirect = 1;
        argv[argc - 2] = NULL;
        outfile = argv[argc - 1];
    }
    else
        cp_redirect = 0;

    if (strcmp(argv1[0], "quit") == 0) {
        free(root);
        exit(0);
    }
    
    if(strcmp(argv[0], "history") == 0){
        for(i = 0; i < 20; i++){
            printf("%s \n", history[i]);
        }
        return;
    }

    if(argc == 3 && argv[0][0] == '$' && !strcmp(argv[1], "=")){
        strcpy(root[size_of_map].key , argv[0]);
        strcpy(root[size_of_map].value, argv[2]);
        size_of_map++;
    }


    /* print the status code of the last command */
    if(argc >= 2 && !strcmp(argv[0], "echo")){
        if(!strcmp(argv1[1], "$?")){
            printf("(%d)\n", status);
            return;
        }
        else{
            exevp_flag =1;
        }
    }

    if (argc ==2 && !strcmp(argv[0], "cd")){
        if(chdir(argv[1]) == -1)
            perror("cd error");
    }
    if (argc ==2 && !strcmp(argv[0], "read")){
        char temp [1024] ={0};
        scanf("%s", temp);
        root[size_of_map].key[0] = '$';
        strcpy(root[size_of_map].key + 1, argv[1]);
        strcpy(root[size_of_map].value, temp);
        size_of_map++;
        wait(NULL);
        return;

    }
    if (fork() == 0) { 

        /* redirection of error IO */
        if (error_redirect) {
            fd = creat(error_outfile, 0660); 
            close (STDERR_FILENO) ; 
            dup(fd); 
            close(fd); 
            /* stderr is now redirected */
        }
        /* redirection of IO ? */
        if (redirect) {
            fd = creat(outfile, 0660); 
            close (STDOUT_FILENO) ; 
            dup(fd); 
            close(fd); 
            /* stdout is now redirected */
        }
        /* copy the IO to the end of exist file */
        if (cp_redirect) {
            fd = open(outfile, O_CREAT | O_WRONLY | O_APPEND, 0660);
            close(STDOUT_FILENO);
            dup(fd);
            close(fd);
            
        }
        // if the thread of execvp not wotking, close the thread 
        if(exevp_flag){
            int printer;
            for(int i=1;i<argc;i++){
                printer =1;
                for(int j=0; j<size_of_map;j++){
                    if(!strcmp(argv[i], root[j].key)){
                        printf("%s ", root[j].value);
                        j = size_of_map;
                        printer =0;
                    }
                }
                if(printer){
                    printf("%s ", argv[i]);
                    printer =0;
                }
            }
            printf("\n");
            exit(0);
        }
          
        if(!exevp_flag){
        if (execvp(argv[0], argv) == -1){
            exit(0);
            perror("execvp error");
            printf("\n");
            }
        }


    }
    /* parent continues over here... */
    /* waits for child to exit if required */
    if (amper == 0)
        retid = wait(&status);
}
void pipe_command(map* root,char* command2)
{
    int fd[10][2];
    int i, count=0,argc3;
    char *argv3[10];
    for(i=0;i<strlen(command2);i++)
    {
        if(command2[i]=='|')
            count++;
    }
    int first=0, last=count-1;
    char* commands[1024];
    char *pipe_token = strtok(command2,"|");
    i=0;
    while(pipe_token)
    {
        commands[i] = pipe_token;
        pipe_token = strtok(NULL,"|");
        i++;
    }

    for(i=0;i<count+1;i++)
    {
        token = strtok(commands[i]," ");
        int j=0;
        while (token != NULL)
        {
            argv3[j] = token;
            token = strtok (NULL, " ");
            j++;
        }
        argv3[j] = NULL;
        argc3=j;
        if(i != count){
            if(pipe(fd[i])<0)
            {
                perror("cant creat pipe");
                return;
            }
        }
        if (fork() == 0) { 
            /* first component of command line */ 
            if(i!=count)
            {
                dup2(fd[i][1],1); 
                close(fd[i][0]); 
                close(fd[i][1]); 
            }
            if(i!=first)
            {
                dup2(fd[i-1][0],0); 
                close(fd[i-1][1]); 
                close(fd[i-1][0]); 
            }
            /* stdout now goes to pipe */ 
            /* child process does command */ 
            exe_c(root,argv3,argc3);
            exit(0);
        } 
            /* 2nd command component of command line */ 
        if(i!=first)
        {
            close(fd[i-1][0]); 
            close(fd[i-1][1]); 
        }
        wait(NULL);
            /* standard input now comes from pipe */ 
    }

}