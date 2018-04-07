#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <sys/reg.h>


using namespace std;

int main(){
    pid_t pid;
    switch (pid = fork()){
        case -1:
            exit(EXIT_FAILURE);
            break;

        case 0: //子进程
            ptrace(PTRACE_TRACEME, 0, NULL, NULL);
            execve("/home/wulei/CLionProjects/judgetest/program/1", NULL, NULL);

        default: //父进程
            while(1){
                int status;
                int flag = 0;
                wait(&status);
                if(WIFEXITED(status))
                    return 0;
                if(flag == 0){
                    long syscall_id = ptrace(PTRACE_PEEKUSER, pid, ORIG_EAX* 4);
                    cout << "Process system call ID = " << syscall_id;
                    flag = 1;
                }
                if(flag == 1){
                    int return_value = ptrace(PTRACE_PEEKUSER, pid,  EAX* 4);
                    cout << " with return value = " << return_value << endl;
                    flag = 0;
                }
                ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
            }

    }
}