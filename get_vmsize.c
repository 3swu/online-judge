#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>

#define BUF_MAX_SIZE 200

void substr(char* dest, char* src, int start, int end);

int main(){
    int fd = open("/home/wulei/status", O_RDONLY, 0644);
    char buf[BUF_MAX_SIZE] = {0};
    char substr_buf[10] = {0};
    if(fd == -1){
        printf("open file error\n");
        exit(1);
    }
    while(!feof(fd)){
        memset(buf, '\n', BUF_MAX_SIZE);
        memset(substr_buf, '\n', 10);
        fgets(buf, BUF_MAX_SIZE - 1, fd);
        substr(substr_buf, buf, 0 ,5);
        if(substr_buf == "VmSize"){
            // char vmsize_str[10] = {0};
            char* kB_loc = strstr(buf, "kB");
            int i = 0;
            for(; &buf[i+2] != kB_loc; ++i);
            substr(substr_buf, buf, 10, i);
            int size = atoi(substr_buf);
            printf("%d\n", size);
            
        }

    }
}

void substr(char* dest, char* src, int start, int end){
    int j = 0;
    for(int i=start; i<=end; ++i, ++j){
        dest[j] = src[i];
    }
    dest[++j] = '\n';
}

// int main(){
//     char a[10] = {0};
//     char b[100] = "VmSize:	  923876 kB";
//     char* p = strstr(b, "kB");
//     int i = 0;
//     for(i=0; &b[i+2] != p; ++i);
//     substr(a, b, 10, i);
//     int size = atoi(a);
//     printf("%d\n", size);

// }
