/**
 * File:"get_procress_memsize.h"
 * Author:leiwu
 * E-mail:wuleiatso@gmail.com
 * Description:Get a process' virtual memory size in file---"/proc/{pid}/status"
 */
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#define BUF_MAX_SIZE 200

void substr(char* dest, char* src, int start, int end);

int get_process_memsize(int pid){
    char pid_str[20] = {'\n'};
    sprintf(pid_str, "%d", pid);
    char status_file_path[100] = {'\n'};
    sprintf(status_file_path, "/proc/%s/status", pid_str);

    FILE* fd = fopen(status_file_path, "r");
    char buf[BUF_MAX_SIZE] = {0};
    char substr_buf[10] = {0};
    if(fd == NULL){
        printf("open file error\n");
        // exit(1);
        return 0;
    }

    while(!feof(fd)){
        memset(buf, '\n', BUF_MAX_SIZE);
        //memset(substr_buf, '\n', 10);
        fgets(buf, BUF_MAX_SIZE - 1, fd);
        substr(substr_buf, buf, 0 ,5);
        char c[10] = "VmSize";
        if(strcmp(substr_buf, c) == 0){
            // char vmsize_str[10] = {0};
            char* kB_loc = strstr(buf, "kB");
            int i = 0;
            for(; &buf[i+2] != kB_loc; ++i);
            int j = i;
            for(; buf[j] != ' '; j--);
            substr(substr_buf, buf, j, i);
            int size = atoi(substr_buf);
            return size;

        }

    }
    return 0;
}

void substr(char* dest, char* src, int start, int end){
    int j = 0;
    for(int i=start; i<=end; ++i, ++j){
        dest[j] = src[i];
    }
    dest[++j] = '\n';
}

// int main(){
//     printf("%d\n", get_process_memsize(31331));
//
// }
