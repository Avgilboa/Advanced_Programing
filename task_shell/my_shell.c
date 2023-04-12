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
#define SIGINT 2

/*include handler for cntrl c */
void sig_hand(int dummy){
    printf("\nyou pressed ctrl-c!\n");
    return;
}
int main() {
signal(SIGINT, sig_hand);
char history[20][1024] = {""};
char command[1024];
char *token;
int i, history_index = 0;
char *outfile;
char* error_outfile;
char * promt = "hello";
int fd, amper, cp_redirect ,redirect, error_redirect, piping, retid, status, argc1;
int fildes[2];
int flag =1;
char *argv1[10], *argv2[10];
int counter;
// The handler of the cntrl + c 

while (1)
{
    if (flag){
        printf("%s: ", promt);
        fgets(command, 1024, stdin);
        command[strlen(command) - 1] = '\0';
    }
    flag =1;
    strcpy(history[history_index] , command);
    history_index = ( history_index + 1 ) %20;
    piping = 0;

    /* parse command line */
    i = 0;
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
        i = 0;
        while (token!= NULL)
        {
            token = strtok (NULL, " ");
            argv2[i] = token;
            i++;
        }
        argv2[i] = NULL;
    }
    
    if (argc1 == 1 && command[0] == '\033') {
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
            continue;
        }
        else{
            strcpy(command, history[(history_index - counter_arrow) % 20]);
            flag =0;
            continue;
        }

    }

    if(argc1 == 1 && !strcmp(argv1[0], "!!")){
        strcpy(command, history[(history_index - 2) % 20]);
        printf("%s\n",history[(history_index - 2)% 20]);
        history_index = (history_index - 1)% 20;
        //strcpy(history[history_index] , command);
        flag =0;
        continue;
    }

    if (argc1 == 3 && ! strcmp(argv1[0], "prompt") && ! strcmp(argv1[1], "=")) {
        promt = argv1[2];
        continue;
    }
    /* Does command line end with & */ 
    if (! strcmp(argv1[argc1 - 1], "&")) {
        amper = 1;
        argv1[argc1 - 1] = NULL;
        }
    else 
        amper = 0; 
    
    /* errors redirecting */
    if (argc1 > 1 && ! strcmp(argv1[argc1 - 2], "2>")) {
        error_redirect = 1;
        argv1[argc1 - 2] = NULL;
        error_outfile = argv1[argc1 - 1];
        argc1 = argc1 - 2;
        }
    else 
        error_redirect = 0;

    /* redirection of IO*/
    if (argc1 > 1 && ! strcmp(argv1[argc1 - 2], ">")) {
        redirect = 1;
        argv1[argc1 - 2] = NULL;
        outfile = argv1[argc1 - 1];
        }
    else 
        redirect = 0; 

    /* copy the IO to the end of exist file */
    if(!redirect && argc1 > 1 && ! strcmp(argv1[argc1 -2 ], ">>")){
        cp_redirect = 1;
        argv1[argc1 - 2] = NULL;
        outfile = argv1[argc1 - 1];
    }
    else
        cp_redirect = 0;

    if (strcmp(argv1[0], "quit") == 0) {
        exit(0);
    }
    
    if(strcmp(argv1[0], "history") == 0){
        for(i = 0; i < 20; i++){
            printf("%s \n", history[i]);
        }
        continue;
    }

    /* print the status code of the last command */
    if(argc1 ==2 && !strcmp(argv1[0], "echo")){
        if(!strcmp(argv1[1], "$?")){
            printf("(%d)\n", status);
            continue;
        }

    }

    if (argc1 ==2 && !strcmp(argv1[0], "cd")){
        if(chdir(argv1[1]) == -1)
            perror("cd error");
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

        if (piping) {
            pipe (fildes);
            if (fork() == 0) { 
                /* first component of command line */ 
                close(STDOUT_FILENO); 
                dup(fildes[1]); 
                close(fildes[1]); 
                close(fildes[0]); 
                /* stdout now goes to pipe */ 
                /* child process does command */ 
                execvp(argv1[0], argv1);
            } 
            /* 2nd command component of command line */ 
            close(STDIN_FILENO);
            dup(fildes[0]);
            close(fildes[0]); 
            close(fildes[1]); 
            /* standard input now comes from pipe */ 
            execvp(argv2[0], argv2);
        } 
        else
        // if the thread of execvp not wotking, close the thread 
        if (execvp(argv1[0], argv1) == -1){
            exit(0);
            perror("execvp error");
            printf("\n");
        }
    }
    /* parent continues over here... */
    /* waits for child to exit if required */
    if (amper == 0)
        retid = wait(&status);
}
}
