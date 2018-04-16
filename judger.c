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
#include <time.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include "get_process_memsize.h"
//#include "error_and_result.h"
#include "outfile_compare.h"


#define FIFO_STDIN     "fifo_stdin"     //定义父子进程管道文件名
#define FIFO_STDOUT    "fifo_stdout"
#define FIFO_STDERR    "fifo_stderr"

//全局变量
char* infile_name = NULL;
char* outfile_name = NULL;
char* temdir_path = NULL;
char* stdin_name = NULL;
char* stdout_name = NULL;
char* stderr_name = NULL;
char* std_ans = NULL;

int time_limit, memory_limit;
int file_type = 0;//语言类别 1:C/2:C++

pid_t parent_pid, child_pid;
uid_t parent_uid;
gid_t parent_gid;

//函数声明
void init_variable(char**);
void prog_compile();
// void error(char*, int);
void execute_prog();
// void return_result(char*, int);
int syscall_illegal(int);

int LANG_CV[256] = {0,1,2,3,4,5,8,9,11,12,20,21,59,63,89,158,231,240, SYS_time, SYS_read, SYS_uname, SYS_write, SYS_open,
                    SYS_close, SYS_execve, SYS_access, SYS_brk, SYS_munmap, SYS_mprotect,
                    SYS_mmap, SYS_fstat, SYS_set_thread_area, 252, SYS_arch_prctl, 0 };

/* 初始化全局变量 */
void init_variable(char** params){
    infile_name = params[0];
    outfile_name = params[1];
    temdir_path = params[2];
    stdin_name = params[3];
    stdout_name = params[4];
    stderr_name = params[5];
    std_ans = params[6];
    time_limit = atoi(params[7]);
    memory_limit = atoi(params[8]);
    file_type = atoi(params[9]);
}

/* 编译源文件 */
void prog_compile(){
    char* infile_path = (char*) malloc (100 * sizeof(char));
    if(file_type == 0){
        error("no language type", 0);
    }
    else if(file_type == 1){ //C编译
        char cmd[100];
        sprintf(infile_path, "%s/%s", temdir_path, infile_name);
        char* outfile_path = (char*) malloc (100 * sizeof(char));
        sprintf(outfile_path, "%s/%s", temdir_path, outfile_name);
        sprintf(cmd, "gcc -O2 -w -static -fmax-errors=3 -std=c11 %s -lm -o %s", infile_path, outfile_path);
        system(cmd);
        
        if(access(outfile_path, 0) != 0) // 编译失败 文件不存在 
            return_result("compile error", RS_CE);
    
    }
    else if(file_type == 2){ //C++编译
        char cmd[100];
        sprintf(infile_path, "%s/%s", temdir_path, infile_name);
        char* outfile_path = (char*) malloc (100 * sizeof(char));
        sprintf(outfile_path, "%s/%s", temdir_path, outfile_name);
        sprintf(cmd, "g++ -O2 -w -static -fmax-errors=3 -std=c++14 %s -lm -o %s", infile_path, outfile_path);
        system(cmd);
        if(access(outfile_path, 0) != 0)
            return_result("compile error", RS_CE);
    }

}

void execute_prog(){
    int status;
    child_pid = fork();
    if(child_pid == -1)
        error("fork error", ERR_FE);

    /* 子进程 */
    if(child_pid == 0){
//        printf("%s\n%s\n", temdir_path, outfile_name);

        if(chroot(temdir_path) != 0)
            //perror("chroot");
            error("chroot error", ERR_CRE);
        chdir("/");

        //标准输入输出错误重定向到文件
//        if(mkfifo(FIFO_STDIN, 0777)<0 || mkfifo(FIFO_STDOUT, 0777)<0 || mkfifo(FIFO_STDERR, 0777)<0)
//            error("mkfifo error", ERR_MFFE);
//    
        // printf("%s\n%s\n%s\n", stdin_name, stdout_name, stderr_name);

        int fd_in, fd_out, fd_err;
        fd_in = open(stdin_name, O_RDONLY);
        fd_out = open(stdout_name, O_WRONLY);
        fd_err = open(stderr_name, O_WRONLY);
        if(fd_in == -1 || fd_out == -1 || fd_err == -1)
            error("open fifo error", ERR_OFFE);
        dup2(fd_in, 0);
        dup2(fd_out, 1);
        dup2(fd_err, 2);

        // struct rlimit r;
        // getrlimit(RLIMIT_CPU, &r);
        // r.rlim_cur = r.rlim_max = time_limit / 1000;
        // setrlimit(RLIMIT_CPU, &r);
        // getrlimit(RLIMIT_AS, &r);
        // r.rlim_cur = r.rlim_max = memory_limit * 1024 * 1.25;
        // setrlimit(RLIMIT_AS, &r);

//        char cwd[100] = {'\0'};
//        getcwd(cwd, 100);
//        printf("%s\n", cwd);

        if(ptrace(PTRACE_TRACEME, 0, NULL, NULL) < 0)
            error("ptrace error", ERR_PTE);

        // char path[100] = {'\n'};
        // sprintf(path, "%s/%s", temdir_path, outfile_name);

        if(execv(outfile_name, NULL) == -1)
//            printf("execv error\n");
//            perror("execv");
            error("execv error", ERR_ER);

    }

        /*父进程*/
    else{
        struct rusage usage;
        // struct meminfo m;
        time_t time_usage = 0;
        int mem_usage = 0;
        while(1){
            // wait3(&status, WUNTRACED, &usage);

            int f= get_process_memsize(child_pid);
            if(f != 0)
                mem_usage = f;

            wait4(child_pid, &status, WUNTRACED, &usage);

            time_usage = usage.ru_utime.tv_sec * 1000 + usage.ru_utime.tv_usec / 1000 + usage.ru_stime.tv_sec * 1000 + usage.ru_stime.tv_usec / 1000;


            if(time_usage > time_limit){
                kill(child_pid, SIGKILL);
                return_result("Time Limit Exceeded", RS_TLE);
            }
            if(mem_usage > memory_limit){
                kill(child_pid, SIGKILL);
                return_result("Memory Limit Exceeded", RS_MLE);
            }

            if(WIFEXITED(status)){ //正常退出
                char out_path[100] = {'\0'}, ans_path[100] = {'\0'};
                sprintf(out_path, "%s/%s", temdir_path, stdout_name);
                sprintf(ans_path, "%s/%s", temdir_path, std_ans);
                
                if(file_compare(out_path, ans_path)){ //答案与标准不同
                    printf("Output Limit Exceeded\n");
                    return_result("Output Limit Exceeded", RS_OLE);
                }
                printf("Accepted\n");
                return_result("Accepted", RS_AC);

                printf("child process exit normally with signal %d", WEXITSTATUS(status));
                exit(0);
            }
            else if(WIFSIGNALED(status)){ //信号退
                if(WTERMSIG(status) == SIGSEGV){
                    if(mem_usage > memory_limit)
                        return_result("Memory Limit Exceeded", RS_MLE);
                }
                else if(WTERMSIG(status) == SIGXCPU){
                    if(time_usage > time_limit)
                        return_result("Time Limit Exceeded", RS_TLE);
                }
                else if(WTERMSIG(status) == SIGKILL || WTERMSIG(status) == SIGABRT){
                    if(mem_usage > memory_limit)
                        return_result("Memory Limit Exceeded", RS_MLE);
                    else if(time_usage > time_limit)
                        return_result("Time Limit Exceeded", RS_TLE);
                    else
                        return_result("Runtime Error", RS_RE);
                } else
                    return_result("Runtime Error", RS_RE);
                //return_result("Runtime Error", RS_RE);
                _exit(0);
            }
            //timeval结构体: tv_sec(秒数), tv_usec(微秒)
            // time_usage = usage.ru_utime.tv_sec * 1000 + usage.ru_utime.tv_usec / 1000 + usage.ru_stime.tv_sec * 1000 +usage.ru_stime.tv_usec / 1000;
            // mem_usage = get_process_memsize(child_pid);


            long syscall_id;
#ifdef __x86_64__
            syscall_id = ptrace(PTRACE_PEEKUSER, child_pid, 8 * ORIG_RAX, NULL);
#else
            syscall_id = ptrace(PTRACE_PEEKUSER, child_pid, 4 * ORIG_REX, NULL);
#endif

            //判断非法系统调用
            if(syscall_id != -1 && syscall_illegal(syscall_id)){
                printf("Illegal system call: %d\n", syscall_id);
                kill(child_pid, SIGKILL);
                return_result("Runtime Error", RS_RE);
            }else{
            }
            //继续监视子进程
            ptrace(PTRACE_SYSCALL, child_pid, NULL, NULL);

        }

    }

}


/* 判断系统调用是否非法 */
int syscall_illegal(int callid){
    int white_list_length = 256;
    for(int i=0; i<white_list_length; ++i){
        if(LANG_CV[i] == callid)
            return 0; //合法
    }
    return 1; //非法
}

int main(int args, char* argv[]){
    char* params[10] = {"\0"};
    for(int i=1; i<12; ++i){
        params[i-1] = argv[i];
    }
    init_variable(params);
    prog_compile();
    execute_prog();
}