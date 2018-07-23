#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>


int main(int argc, char *argv[]){
    FILE *fp;
    int flag = 0;
    int i;
    int pid, status;
    char command[256];
    char *tok[7];
    char *tmptok;
    if (argc == 1){ //interactive mode
        while(1){
        i = 0;
        if(flag) break;
        printf("Prompt >> ");
        fgets(command, 256, stdin);
        if(feof(stdin) || command == NULL) break;
        tmptok = strtok(command, " \n");
            while(tmptok!=NULL){
                i = 0;
                while(tmptok!=NULL && strcmp(tmptok, ";")){//tokening
                    tok[i++] = tmptok;
                    tmptok = strtok(NULL, " \n");
                }
                tok[i] = NULL;
                if (!strcmp(tok[0], "quit") || feof(stdin) || tok[0] == NULL){
                    flag = 1;
                    break;
                }
                if((pid = fork()) == -1){
                    perror("fork error");
                    exit(0);
                }
                else if (pid == 0 ){
                    execvp(tok[0], tok);
                }
                else {
                    pid = wait(&status);
                }
                if(tmptok!=NULL) tmptok = strtok(NULL, " \n");
            }
        }
    }
    else {
        fp = fopen(argv[1], "r");
        if(fp == NULL){printf("File Open Error\n"); return 0;}
        while(!feof(fp)){
            if (flag) break;
            fgets(command, 256, fp);
            i = 0;
            tmptok = strtok(command, " \n");
             while(tmptok!=NULL){
                 i = 0;
                 while(tmptok!=NULL && strcmp(tmptok, ";")){//tokening
                     tok[i++] = tmptok;
                     tmptok = strtok(NULL, " \n");
                 }
                 tok[i] = NULL;
                 if (!strcmp(tok[0], "quit") || feof(fp) || tok[0] == NULL){
                     flag = 1;
                     break;
                 }
                 if((pid = fork()) == -1){
                     perror("fork error");
                     exit(0);
                 }
                 else if (pid == 0 ){
                     execvp(tok[0], tok);
                 }
                 else {
                     pid = wait(&status);
                 }
                 if(tmptok!=NULL)tmptok = strtok(NULL, " \n");
             }
        }
        fclose(fp);
    }
    return 0;
}
