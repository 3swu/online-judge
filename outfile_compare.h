#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include "error_and_result.h"

#define LINE_BUF_MAX_SIZE 200

int buf_compare(char* buf1, char* buf2){
    int i;
    for(i = LINE_BUF_MAX_SIZE-1; buf1[i] != '\n'; --i);
    if(i > 0)
        buf1[i] = '\000';
    for(i = LINE_BUF_MAX_SIZE-1; buf2[i] != '\n'; --i);
    if(i > 0)
        buf2[i] = '\000';
    for(i=0; i<LINE_BUF_MAX_SIZE; ++i){
        if(buf1[i] != buf2[i])
            return 1;
    }
    return 0;
}

int file_compare(char* outfile_path, char* stdfile_path){
    FILE* fd_outfile;
    FILE* fd_stdfile;
    fd_outfile = fopen(outfile_path, "r");
    fd_stdfile = fopen(stdfile_path, "r");

    if(fd_outfile == NULL || fd_stdfile == NULL)
        error("Open File Error", ERR_OFE);

    char outfile_buf[LINE_BUF_MAX_SIZE] = {0}, stdfile_buf[LINE_BUF_MAX_SIZE] = {0};
    while(!feof(fd_outfile) && !feof(fd_stdfile)){
        memset(outfile_buf, 0, LINE_BUF_MAX_SIZE);
        memset(stdfile_buf, 0, LINE_BUF_MAX_SIZE);

        fgets(outfile_buf, LINE_BUF_MAX_SIZE -1, fd_outfile);
        fgets(stdfile_buf, LINE_BUF_MAX_SIZE -1, fd_stdfile);
        if(buf_compare(outfile_buf, stdfile_buf)){
            fclose(fd_outfile);
            fclose(fd_stdfile);
            return 1;
        }
            
    }
    fclose(fd_outfile);
    fclose(fd_stdfile);
    return 0;
}
// int main(){
//     printf("%s\n", file_compare("/home/wulei/test/1.out", "/home/wulei/test/1.std") ? "file different" : "same");
// }