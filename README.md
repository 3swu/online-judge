## About

+ 这是一个Online Judge系统的判题部分
+ 运行在独立的服务器上，通过Socket与Web端通信
+ 关于返回结果码的意义定义在`error_and_result.h`头文件中。

## Usage

+ Clone这个项目
+ 在`config`文件第一行设置服务器端口
+ 在`config`文件第二行设置判题临时文件存放路径
+ 编译`judger.c`文件为`judger`
+ 编译`judge-server.c`文件为`judge-server`
+ 以超级用户权限运行`judge-server`
