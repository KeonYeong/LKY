#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char * argv[]){
    int pid, i;
    if((pid = fork()) < 0){
        printf (1,"fork error\n");
        exit();
    }
    else if (pid == 0){
        for(i =0;i<100;i++){
            printf(1,"child\n");
            yield();
        }
        exit();
    }
    else if (pid > 0){
        for(i=0;i<100;i++){
            printf(1,"parent\n");
            yield();
        }
    }
    exit();
}


