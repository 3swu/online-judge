#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <sys/reg.h>
#include <string.h>
#include <fcntl.h>
#include <sys/resource.h>

int main(){
    pid_t child_pid = fork();
    int status;
    if(child_pid == 0){
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        printf("Child process\n");
        exit(5);
    }
    else{
        struct rusage usage;
        while(1){
            wait4(child_pid, &status, WUNTRACED, &usage);
            //wait3(&status, WUNTRACED, &usage);
            // if(WIFEXITED(status))
            //     return 0;
            int time_usage = usage.ru_utime.tv_sec * 1000 + usage.ru_utime.tv_usec / 1000 + usage.ru_stime.tv_sec * 1000 + usage.ru_stime.tv_usec / 1000;
            int mem_usage = usage.ru_maxrss / 1024;//KB
            printf("time_usage = %d\nmem_usage = %d\n", time_usage, mem_usage);
            ptrace(PTRACE_SYSCALL, child_pid, NULL, NULL);
        }
    }
}