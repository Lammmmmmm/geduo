<!--
 * @file: 
 * @brief: 
 * @author: Choupin
 * @email: 593214609@qq.com
 * @date: Do not edit
 * @LastEditTime: 2020-06-14 21:12:16
 * @LastEditors: Choupin
--> 
# 项目介绍

## 0 开发目的

该项目的开发初衷是对`C++`、`Linux`系统编程和网络编程相关知识的实战练习。

而练习自然也该有个目标，本人对后端开发比较感兴趣，`C++`的各种网络库不少，值得阅读源码去学习的不多，关键我也不知道以后工作会用什么库，索性就自己跟着撸一个，去理解一个框架如何设计，这个过程中又需要考虑什么。我认为这也许对以后工作了快速上手一个其他框架会有帮助。

而这个项目之前在本地电脑上有实现了几个模块，但并不完整，实现也十分简陋。最近在某`go`语言大厂实习，担心自己`C++`长时间不写过于生疏，决定利用晚上和周末的时间，将这个项目再重头写一遍，同时也希望能对之前所学有个好好的梳理。

本项目参考自某鹅厂大佬的`SYLAR`框架，**仅做学习使用**。

## 1 开发环境

`Ubuntu 18.04`

`gcc (Ubuntu 7.5.0-3ubuntu1~18.04) 7.5.0`

`cmake 3.10.2`

## 2 主要特色

- 封装`pthread`，实现线程池，支持读写锁、自旋锁等

- 以`ucontext`实现协程，并使用线程池实现协程调度系统，`N-M`的协程调度模型，支持协程在线程中切换，可重复利用每一个线程

- 使用`epoll`实现定时器，支持Socket读写时间的添加，删除，取消功能。支持一次性定时器，循环定时器，条件定时器等功能

- 选择`YAML`作为配置文件，支持`STL`容器数据类型，定义即可用，无需单独解析，支持变更通知功能。

- 实现日志模块，支持流式风格和格式化风格，能够对日志内容、格式、级别能自由配置

- 实现二进制数据的序列化和反序列化，`std::string`的读写支持,支持字节序转化,支持序列化到文件，以及从文件反序列化等功能

- 封装`Socket`，并在此基础上实现了通用的`TcpServer`的服务器类，提供简单的API，使用便捷，可以快速绑定一个或多个地址，启动服务，监听端口，accept连接，处理socket连接等功能，作为具体业务的父类。

- 将文件、`Socket`封装统一的流式接口，实现SocketStream；在此基础上，采用Ragel（有限状态机，性能媲美汇编），实现了HTTP/1.1的简单协议实现和uri的解析。

## 3 具体模块介绍

### 3.1 协程

## 4 联系方式

`QQ`：`593214609`
`e-mail`：`ozc236@163.com`
个人网站：`www.choubin.site`