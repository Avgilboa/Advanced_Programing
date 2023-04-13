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


int main() {
int size_of_map = 0;
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
int exevp_flag = 0;
int add_varb = 0;
map* root = (map*)malloc(1024 * sizeof(map));
int ifi=0;
char then_command[1024];
char else_command[1024];
// The handler of the cntrl + c 

while (1)
{
    ifi =0;
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
        free(root);
        exit(0);
    }
    
    if(strcmp(argv1[0], "history") == 0){
        for(i = 0; i < history_index; i++){
            printf("(%d) %s \n",i+1 , history[i]);
        }
        continue;
    }

    if(argc1 == 3 && argv1[0][0] == '$' && !strcmp(argv1[1], "=")){
        strcpy(root[size_of_map].key , argv1[0]);
        strcpy(root[size_of_map].value, argv1[2]);
        size_of_map++;


    }
    ///// if condition <--
    /// then
    // command  <--
    // else
    // command <--
    // fi
    if( !strcmp(argv1[0], "if")){
        char fi_command[1024];
        scanf("%s", then_command);
        if(! strcpy(then_command, "then")){
            fgets(then_command, 1024, stdin);
            then_command[strlen(then_command) - 1] = '\0';
            }
            else{continue;}
            scanf("%s", else_command);
            if(!strcpy(else_command, "else")){
                fgets(else_command, 1024, stdin);
                else_command[strlen(else_command) - 1] = '\0';
                }
                else{continue;}
                scanf("%s", fi_command);
                if(strcmp(fi_command, "fi")){continue;}
                flag =0;
        }
    /* print the status code of the last command */
    if(argc1 >= 2 && !strcmp(argv1[0], "echo")){
        if(!strcmp(argv1[1], "$?")){
            printf("(%d)\n", status);
            continue;
        }
        else{
            exevp_flag =1;
        }
}

    if (argc1 ==2 && !strcmp(argv1[0], "cd")){
        if(chdir(argv1[1]) == -1)
            perror("cd error");
    }
    if (argc1 ==2 && !strcmp(argv1[0], "read")){
        char temp [1024] ={0};
        scanf("%s", temp);
        root[size_of_map].key[0] = '$';
        strcpy(root[size_of_map].key + 1, argv1[1]);
        strcpy(root[size_of_map].value, temp);
        size_of_map++;
        wait(NULL);
        continue;

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
        if(exevp_flag){
            int printer;
            for(int i=1;i<argc1;i++){
                printer =1;
                for(int j=0; j<size_of_map;j++){
                    if(!strcmp(argv1[i], root[j].key)){
                        printf("%s ", root[j].value);
                        j = size_of_map;
                        printer =0;
                    }
                }
                if(printer){
                    printf("%s ", argv1[i]);
                    printer =0;
                }
            }
            printf("\n");
            exit(0);
        }

        if(!exevp_flag ){
            printf("YOU ARE HERE\n");
        if (execvp(argv1[0], argv1) == -1){
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
}
