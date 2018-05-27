## About

+ 这是一个Online Judge系统的判题部分
+ 运行在独立的服务器上，通过Socket与Web端通信
+ 关于返回结果码的意义定义在`error_and_result.h`头文件中

## Usage

+ Clone这个项目
+ 在`config`文件第一行设置服务器端口
+ 在`config`文件第二行设置判题临时文件存放路径
+ 在项目文件夹中运行`sudo make`命令来构建项目
+ 运行`sudo judge-server`

## Protocol

评测机与Web端通信应遵守以下协议，且应采取小端编码(从低字节开始编码)，所有未编码的空字节填充0（‘\000’）， 传输顺序如下：

1. Web端向评测机发起TCP连接请求以建立Socket连接，开始向评测机传输数据：

   在这个阶段的传输中，以512字节组成的字节串为一个单位（以下简称一个“块”），所有的块按顺序连接组成最后传输的串，最后的串长度应是512的整数倍

   + 第一个块

     + 0-19字节

       username：用户的用户名

     + 20-24字节

       problem_id：题目的编号

     + 25-29字节

       type：代码类型，1：C，2：C++

     + 30-39字节

       source_len：源代码的字节数

     + 40-49字节

       in_len：输入文件的字节数

     + 50-59字节

       answer_len：输出文件的字节数

     + 60-69字节

       time_limit：时间限制（以ms为单位）

     + 70-79字节

       memory_limit：内存限制（以kB为单位）

     + 80-89字节

       block_nums：传输的字节块数

     + 余下空字节填充0

   + 第二个块开始，按顺序将用户源代码文件，输入文件，输出文件编码到字节串中，且每个文件开头编码在一个新的块中，即每个文件编码的最后一个块中最后余下的部分均填充0，不编码下一个文件

2. 第一阶段的传输完成，此时Web端应立即开始（阻塞）接收评测机的返回结果，返回结果格式在下文，并应设置超时时限（按情况设置，推荐设置为4-6秒）

3. 评测机评测完成，返回结果码，结果码意义定义在上文提到的头文件中，返回串的长度为5字节（通常最后两个字节填充为空字节）

4. 传输完成，关闭连接

## Example
假设在某个目录中已经存在一个需要评测的源文件、一个输入文件、一个答案文件，分别命名为`1.c`, `1.in`, `1.ans`,并且评测端已经运行，则在该目录下使用以下Python程序可以测试：
```python
import socket
import struct
import os
import time
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

s.connect(("127.0.0.1", 6666))

username = "wulei"
pro_id = "1000"
type = "1"
time_limit = "100"
mem_limit = "500000"
source_str = open("1.c", "r").read()
in_str = open("1.in", "r").read()
ans_str = open("1.ans", "r").read()

source_len = os.path.getsize("1.c")
in_len = os.path.getsize("1.in")
ans_len = os.path.getsize("1.ans")

source_blocks = 0
in_blocks = 0
ans_blocks = 0

if(source_len % 512 != 0):
    source_blocks = int(source_len / 512) + 1
else:
    source_blocks = int(source_len / 512)
if(in_len % 512 != 0):
    in_blocks = int(in_len / 512) + 1
else:
    in_blocks = int(in_len / 512)
if(ans_len % 512 != 0):
    ans_blocks = int(ans_len / 512) + 1
else:
    ans_blocks = int(ans_len / 512)

block_nums = source_blocks + in_blocks + ans_blocks + 1

print(source_blocks,in_blocks, ans_blocks)

format =  "<20s5s5s10s10s10s10s10s432s" + str(source_blocks * 512) + "s" + str(in_blocks * 512) + "s" + str(ans_blocks * 512) + "s"
str_package = struct.pack(format, username.encode('utf-8'), pro_id.encode('utf-8'), type.encode('utf-8'), str(source_len).encode('utf-8'), str(in_len).encode('utf-8'), str(ans_len).encode('utf-8'), time_limit.encode('utf-8'), mem_limit.encode('utf-8'), str(block_nums).encode('utf-8'), source_str.encode('utf-8'), in_str.encode('utf-8'), ans_str.encode('utf-8'))
s.send(str_package)

result = s.recv(5)
print(result)

```

## Compile options

评测机对于C或者C++源文件，采用如下的编译方式：

- C：`gcc -O2 -w -static -fmax-errors=3 -std=c11 {file} -lm -o {file}`
- C++：`g++ -O2 -w -static -fmax-errors=3 -std=c++14 {file} -lm -o {file}`

## Author
吴磊(天津师范大学计算机科学与技术专业2015级学生)
[GitHub](https://github.com/wuleiaty)
[Email](wuleiatso@gmail.com)

