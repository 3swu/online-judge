#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <wait.h>
#include <sys/stat.h>
#include <errno.h>

//全局变量
unsigned short port = 0;
char* judge_temp_path = NULL;
char* workspace = NULL;
void parse_config_file(){
    workspace = (char*) malloc (100 * sizeof(char));
    if(getcwd(workspace, 100) == NULL){
        printf("Init workspace failed\n");
        exit(1);
    }

    char config_file_path[110] = {'\0'};
    sprintf(config_file_path, "%s/%s", workspace, "config");
    FILE* config_fd = fopen(config_file_path ,"r");
    if(config_fd == NULL){
        printf("Open configuration file failed\n");
        exit(1);
    }
    char port_buf[10] = {'\0'};
    fgets(port_buf, 9, config_fd);
    port = atoi(port_buf);
    judge_temp_path = (char*) malloc (100 * sizeof(char));
    memset(judge_temp_path, '\0', 100);
    fgets(judge_temp_path, 99, config_fd);
    close(config_fd);

    //输出测试
    printf("port: %d\ntemp path: %s\nworkspace: %s\n", port, judge_temp_path, workspace);

}

void deal_child(int sig_no){
    for(; ; ){
        if(waitpid(-1, NULL, WNOHANG) == 0)
            break;
    }
}


int main() {
    // signal(SIGCHLD, deal_child);
    parse_config_file();

    //初始化套接字描述字
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_fd < 0){
        perror("socket");
        exit(1);
    }

    //设置本地地址结构体
    struct sockaddr_in my_addr;
    bzero(&my_addr, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(port);
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    //绑定
    bind(socket_fd, (struct sockaddr*)&my_addr, sizeof(my_addr));

    //监听
    listen(socket_fd, 0);

    int conn_fd;
    pid_t pid;
    for( ; ; ){
        conn_fd = accept(socket_fd, NULL, NULL);


        //建立子进程
        if((pid = fork()) == 0){
            close(socket_fd);

            int file_source, file_in, file_ans;
            char buf[512] = {'\0'};
            char temp_path[100] = {'\0'};
            char filename[30] = {'\0'};

            int num, i = 0;
            char username[20] = {0};
            int pro_id, type, source_len, in_len, ans_len, time_limit, mem_limit, block_nums = 1;
            int source_start_block, source_end_block, in_start_block, in_end_block, ans_start_block, ans_end_block;


            while(i < block_nums){
                num = recv(conn_fd, buf, sizeof(buf), 0);
                if(i == 0){
                    //解析username
                    int j;
                    for(j=0; j<20; j++){
                        if(buf[j] != 0)
                            username[j] = buf[j];
                    }
                    username[++j] = '\0';

                    //解析pro_id
                    char pro_id_buf[5] = {'\0'};
                    j = 0;
                    for(; j<5; j++){
                        if(buf[20 + j] != 0)
                            pro_id_buf[j] = buf[20 + j];
                    }
                    pro_id = atoi(pro_id_buf);

                    //解析type
                    char type_buf[2] = {'\0'};
                    type_buf[0] = buf[25];
                    type = atoi(type_buf);

                    //解析source_len
                    char source_len_buf[10] = {'\0'};
                    j = 0;
                    for(; j<10; j++){
                        if(buf[30 + j] != 0)
                            source_len_buf[j] = buf[30 + j];
                    }
                    source_len = atoi(source_len_buf);

                    //解析in_len
                    char in_len_buf[10] = {'\0'};
                    j = 0;
                    for(; j<10; j++){
                        if(buf[40 + j] != 0)
                            in_len_buf[j] = buf[40 + j];
                    }
                    in_len = atoi(in_len_buf);

                    //解析ans_len
                    char ans_len_buf[10] = {'\0'};
                    j = 0;
                    for(; j<10; j++){
                        if(buf[50 + j] != 0)
                            ans_len_buf[j] = buf[50 + j];
                    }
                    ans_len = atoi(ans_len_buf);

                    //解析time_limit
                    char time_limit_buf[10] = {'\0'};
                    j = 0;
                    for(; j<10; j++){
                        if(buf[60 + j] != 0)
                            time_limit_buf[j] = buf[60 + j];
                    }
                    time_limit = atoi(time_limit_buf);

                    //解析mem_limit
                    char mem_limit_buf[10] = {'\0'};
                    j = 0;
                    for(; j<10; j++){
                        if(buf[70 + j] != 0)
                            mem_limit_buf[j] = buf[70 + j];
                    }
                    mem_limit = atoi(mem_limit_buf);

                    //解析block_nums
                    char block_nums_buf[10] = {'\0'};
                    j = 0;
                    for(; j<10; j++){
                        if(buf[80 + j] != 0)
                            block_nums_buf[j] = buf[80 + j];
                    }
                    block_nums = atoi(block_nums_buf);


                    //初始化每个文件开始和结尾的缓存块数
                    source_start_block = 1;
                    source_end_block = source_start_block + ((source_len % 512 == 0) ? source_len / 512 -1 : source_len / 512);
                    in_start_block = source_end_block + 1;
                    in_end_block = in_start_block + ((in_len % 512 == 0) ? in_len / 512 - 1 : in_len / 512);
                    ans_start_block = in_end_block + 1;
                    ans_end_block = ans_start_block + ((ans_len % 512 == 0) ? ans_len/ 512 - 1 : ans_len / 512);

                    printf("username: %s\nproblem: %d\n", username, pro_id);

                    //测试输出
//                    printf("%s\n%d\n%d\n%d\n%d\n%d\n", username, pro_id, type, source_len, in_len, ans_len);
//                    printf("%d\n%d\n%d\n%d\n%d\n%d\n", source_start_block, source_end_block, in_start_block, in_end_block, ans_start_block, ans_end_block);
                    //初始化文件名和评测文件夹路径
                    username[1] = '\0';
                    sprintf(filename, "%d%s", pro_id % 10, username);

                    sprintf(temp_path, "%s/%s", judge_temp_path, filename);

                    //新建评测文件夹
                    mkdir(temp_path, 0777);

                    //定义文件路径
                    char temp_file_buf[100] = {'\0'};

                    //初始化源文件
                    sprintf(temp_file_buf, "%s/%s%s", temp_path, filename, (type == 1) ? ".c" : ".cpp");
                    file_source = open(temp_file_buf, O_WRONLY | O_CREAT, 0777);
                    memset(temp_file_buf, '\0', 100);

                    //初始化标准输入文件
                    sprintf(temp_file_buf, "%s/%s%s", temp_path, filename, ".in");
                    file_in = open(temp_file_buf, O_WRONLY | O_CREAT, 0777);
                    memset(temp_file_buf, '\0', 100);

                    //初始化答案文件
                    sprintf(temp_file_buf, "%s/%s%s", temp_path, filename, ".ans");
                    file_ans = open(temp_file_buf, O_WRONLY | O_CREAT, 0777);
                    memset(temp_file_buf, '\0', 100);

                    //新建标准输出文件
                    sprintf(temp_file_buf, "%s/%s%s", temp_path, filename, ".out");
                    int file_out = open(temp_file_buf, O_CREAT, 0777);
                    close(file_out);
                    memset(temp_file_buf, '\0', 100);

                    //新建标准错误文件
                    sprintf(temp_file_buf, "%s/%s%s", temp_path, filename, ".err");
                    int file_err = open(temp_file_buf, O_CREAT, 0777);
                    close(file_err);
                    memset(temp_file_buf, '\0', 100);

                    //free(temp_file_buf);
                }
                else if(i >= source_start_block && i <= source_end_block){
                    if(i == source_end_block)
                        write(file_source, buf, source_len % 512);
                    else
                        write(file_source, buf, 512);
                }
                else if(i >= in_start_block && i <= in_end_block){
                    if(i == in_end_block)
                        write(file_in, buf, in_len % 512);
                    else
                        write(file_in, buf, 512);
                }
                else if(i >= ans_start_block && i <= ans_end_block){
                    if(i == ans_end_block)
                        write(file_ans, buf, ans_len % 512);
                    else
                        write(file_ans, buf, 512);
                }
                ++i;
            }

            close(file_source);
            close(file_in);
            close(file_ans);

            //初始化评测进程参数列表
            char** params = (char**) malloc (12 * sizeof(char*));
            params[11] = NULL;
            char params_buf[100] = {'\0'};

            for(int i=0; i<11; ++i){
                params[i] = (char*) malloc (100 * sizeof(char));
                memset(params[i], '\0', 100);
            }

            sprintf(params_buf, "%s/%s", workspace, "judger");
            strcpy(params[0], params_buf);
            memset(params_buf, '\0', 100);

            sprintf(params_buf, "%s%s", filename, (type == 1) ? ".c" : ".cpp");
            strcpy(params[1], params_buf);
            memset(params_buf, '\0', 100);

            strcpy(params[2], filename);

            strcpy(params[3], temp_path);

            sprintf(params_buf, "%s%s", filename, ".in");
            strcpy(params[4], params_buf);
            memset(params_buf, '\0', 100);

            sprintf(params_buf, "%s%s", filename, ".out");
            strcpy(params[5], params_buf);
            memset(params_buf, '\0', 100);

            sprintf(params_buf, "%s%s", filename, ".err");
            strcpy(params[6], params_buf);
            memset(params_buf, '\0', 100);

            sprintf(params_buf, "%s%s", filename, ".ans");
            strcpy(params[7], params_buf);
            memset(params_buf, '\0', 100);

            sprintf(params_buf, "%d", time_limit);
            strcpy(params[8], params_buf);
            memset(params_buf, '\0', 100);

            sprintf(params_buf, "%d", mem_limit);
            strcpy(params[9], params_buf);
            memset(params_buf, '\0', 100);

            sprintf(params_buf, "%d", type);
            strcpy(params[10], params_buf);
            memset(params_buf, '\0', 100);
     
     

            int status;
            pid_t judge_process_pid = fork();
            // close(conn_fd);

            if(judge_process_pid == 0){
                // close(conn_fd);
                if(execv(params[0], params) == -1){
                    printf("%s\n", strerror(errno));
                }
            }
            else{
                wait(&status);
                if(WIFEXITED(status)){
                    int result_code = WEXITSTATUS(status);
                    printf("code = %d\n\n", result_code);
                    char* result_buf = (char*) malloc (5 * sizeof(char));
                    sprintf(result_buf, "%d", result_code);
                    write(conn_fd, result_buf, 5);
                    
                }
            }

            //删除编译文件(静态编译比较大)
            char compile_file[100] = {'\0'};
            sprintf(compile_file, "%s/%s", temp_path, filename);
            remove(compile_file);
            exit(0);


        }
        close(conn_fd);
    }
}