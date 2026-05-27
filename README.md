# ChatServer - 基于C++的聊天服务器系统

- Linux 集群聊天服务器，实现用户注册登陆、收发消息、多人群聊、在线状态管理、离线消息缓存等功能
- 基于 muduo 网络库开发网络模块，提供高并发网络IO服务，实现高效通信，同时解耦网络和业务模块代码
- 使用第三方 JSON 库实现通信数据的序列化和反序列化，传输 JSON 格式消息
- 使用 Nginx 的 TCP 负载均衡功能，将客户端请求分派到多个服务器上实现集群功能，提高并发处理能力
- 基于 Redis 发布-订阅消息通信模式，实现跨服务器的消息通信
- 使用 MySQL 存储用户数据和消息
- Windows Subsystem for Linux - Ubuntu 24.04 LTS & Visual Studio Code
- CMake 3.28.3 & Redis 7.0.15 & MySQL 8.0.42 & NginX 1.28.0

## 项目简介

ChatServer是一个基于C++开发的高性能聊天服务器系统，采用客户端-服务器架构，支持用户注册、登录、一对一聊天、群组聊天等功能。项目使用muduo网络库实现高并发网络通信，MySQL数据库存储用户数据，Redis实现消息发布订阅机制。

## 功能特性

### 核心功能
- **用户管理**: 用户注册、登录、注销
- **好友系统**: 添加好友、好友列表管理
- **一对一聊天**: 实时私聊功能
- **群组功能**: 创建群组、加入群组、群组聊天
- **离线消息**: 支持离线消息存储和推送
- **在线状态**: 实时显示用户在线/离线状态

### 技术特性
- **高并发**: 基于muduo网络库的Reactor模式
- **多线程**: 主线程监听连接，工作线程处理业务
- **数据库持久化**: MySQL存储用户、好友、群组等数据
- **消息队列**: Redis实现跨服务器消息传递
- **JSON通信**: 使用nlohmann/json进行数据序列化

## 技术栈

- **编程语言**: C++11/14
- **网络库**: muduo (基于epoll的高性能网络库)
- **数据库**: MySQL 8.0
- **缓存**: Redis
- **构建系统**: CMake
- **JSON库**: nlohmann/json
- **线程库**: pthread

## 项目结构

```
ChatServer/
├── bin/                    # 编译生成的可执行文件 
├── build/                  # CMake 构建目录 
├── include/                # 头文件目录
│   ├── client/            # 客户端头文件
│   ├── server/            # 服务器头文件
│   └── public.h           # 公共定义
├── src/                   # 源代码目录
│   ├── client/            # 客户端源码
│   └── server/            # 服务器源码
├── json4cpp/              # JSON 库
├── Makefile               # 顶层构建入口
├── CMakeLists.txt         # CMake 主构建文件
└── init.sql               # 数据库初始化脚本
```

### 核心模块

#### 服务器端
- **ChatServer**: 主服务器类，负责网络连接管理
- **ChatService**: 业务逻辑处理类，单例模式
- **UserModel**: 用户数据模型
- **FriendModel**: 好友关系数据模型
- **GroupModel**: 群组数据模型
- **OfflineMsgModel**: 离线消息数据模型
- **Redis**: Redis操作封装类

#### 客户端
- **main.cpp**: 客户端主程序，提供命令行界面
- **main.h**: 客户端头文件，定义全局变量和函数

## 数据库设计

### 用户表 (user)
```sql
CREATE TABLE `user` (
  `id` int NOT NULL AUTO_INCREMENT,
  `name` varchar(50) DEFAULT NULL,
  `password` varchar(50) DEFAULT NULL,
  `state` enum('online','offline') DEFAULT 'offline',
  PRIMARY KEY (`id`),
  UNIQUE KEY `name` (`name`)
);
```

### 好友表 (friend)
```sql
CREATE TABLE `friend` (
  `userid` int NOT NULL,
  `friendid` int NOT NULL,
  KEY `userid` (`userid`,`friendid`)
);
```

### 群组表 (allgroup)
```sql
CREATE TABLE `allgroup` (
  `id` int NOT NULL AUTO_INCREMENT,
  `groupname` varchar(50) NOT NULL,
  `groupdesc` varchar(200) DEFAULT '',
  PRIMARY KEY (`id`),
  UNIQUE KEY `groupname` (`groupname`)
);
```

### 群组成员表 (groupuser)
```sql
CREATE TABLE `groupuser` (
  `groupid` int NOT NULL,
  `userid` int NOT NULL,
  `grouprole` enum('creator','normal') DEFAULT NULL,
  KEY `groupid` (`groupid`,`userid`)
);
```

### 离线消息表 (offlinemessage)
```sql
CREATE TABLE `offlinemessage` (
  `userid` int NOT NULL,
  `message` varchar(500) NOT NULL
);
```

## 消息协议

### 消息类型 (EnMsgType)
- `LOGIN_MSG (1000)`: 登录消息
- `LOGIN_MSG_ACK`: 登录响应消息
- `LOGOUT_MSG`: 注销消息
- `SIGN_UP_MSG`: 注册消息
- `SIGN_UP_MSG_ACK`: 注册响应消息
- `ONE_CHAT_MSG`: 一对一聊天消息
- `ADD_FRIEND_MSG`: 添加好友消息
- `CREATE_GROUP_MSG`: 创建群组消息
- `CREATE_GROUP_MSG_ACK`: 创建群组响应消息
- `ADD_GROUP_MSG`: 加入群组消息
- `GROUP_CHAT_MSG`: 群组聊天消息

### 错误码 (ErrorCode)
- `PASSWORD_ERROR (2000)`: 密码错误
- `NO_EXIST_USER`: 用户不存在
- `NO_EXIST_Group`: 群组不存在

## 环境要求

### 系统要求
- Linux操作系统 (推荐Ubuntu 20.04+)
- GCC 7.0+ 或 Clang 5.0+
- CMake 3.5+

### 依赖库
- **muduo**: 高性能C++网络库
- **MySQL**: 数据库服务器和客户端库
- **Redis**: 内存数据库和客户端库 (hiredis)
- **pthread**: POSIX线程库

## 安装和构建

### 1. 安装依赖

#### Ubuntu/Debian
```bash
# 安装基础开发工具
sudo apt update
sudo apt install build-essential cmake

# 安装MySQL开发库
sudo apt install libmysqlclient-dev

# 安装Redis开发库
sudo apt install libhiredis-dev

# 安装muduo网络库
sudo apt install libmuduo-dev
```

#### CentOS/RHEL
```bash
# 安装基础开发工具
sudo yum groupinstall "Development Tools"
sudo yum install cmake

# 安装MySQL开发库
sudo yum install mysql-devel

# 安装Redis开发库
sudo yum install hiredis-devel

# 编译安装muduo (需要从源码编译)
```

### 2. 数据库配置

```bash
# 启动MySQL服务
sudo systemctl start mysql

# 创建数据库
mysql -u root -p
CREATE DATABASE chat;
USE chat;

# 导入数据库结构
source init.sql;
```

### 3. 编译项目

#### 使用顶层 Makefile

```bash
cd ChatServer

# 配置并编译
make

# 清理编译产物
make clean

# 完全重新构建
make rebuild
```

#### 手动使用 CMake

```bash
cd ChatServer
mkdir build && cd build
cmake ..
make
```

编译完成后，可执行文件输出到 `bin/` 目录：

```bash
ls bin/
# ChatServer  ChatClient
```

## 使用方法

### 启动服务器

```bash
# 启动聊天服务器 (端口号)
./bin/ChatServer 6666
```

### 启动客户端

```bash
# 连接服务器 (IP地址 端口号)
./bin/ChatClient 127.0.0.1 6666
```

### 客户端命令

启动客户端后，可以使用以下命令：

1. **登录**: 输入用户ID和密码
2. **注册**: 输入用户名和密码
3. **聊天命令**:
   - `help`: 显示所有支持的命令
   - `chat:friendid:message`: 一对一聊天
   - `addfriend:friendid`: 添加好友
   - `creategroup:groupname:groupdesc`: 创建群组
   - `addgroup:groupid`: 加入群组
   - `groupchat:groupid:message`: 群组聊天
   - `logout`: 注销登录

### 使用示例

```
======================login user======================
current login user => id:1 name:suzune
----------------------friend list---------------------
2 chieko online
----------------------group list----------------------
1 testgroup test
1 suzune online creator
2 chieko online normal
======================================================

show command list >>> 
help : 显示所有支持的命令 格式help
chat : 一对一聊天 格式chat:friendid:message
addfriend : 添加好友 格式addfriend:friendid
creategroup : 创建群组 格式creategroup:groupname:groupdesc
addgroup : 加入群组 格式addgroup:groupid
groupchat : 群聊 格式groupchat:groupid:message
logout : 注销 格式logout

chat:2:Hello, how are you?
2024-01-01 12:00:00 [1]suzune said: Hello, how are you?
```

## 架构设计

### 网络架构
- **Reactor模式**: 基于muduo的one loop per thread + threadPool模型
- **主从Reactor**: 主线程负责监听连接，工作线程处理业务逻辑
- **非阻塞I/O**: 使用epoll实现高并发网络通信

### 业务架构
- **单例模式**: ChatService采用单例模式管理业务逻辑
- **MVC模式**: 分离数据模型(Model)、业务逻辑(Service)、网络层(Server)
- **回调机制**: 使用函数对象实现消息处理回调

### 数据流
1. 客户端发送JSON格式消息
2. 服务器解析消息并路由到对应处理函数
3. 业务逻辑处理并更新数据库
4. 通过Redis发布消息给其他服务器实例
5. 返回响应给客户端

## 性能特性

- **高并发**: 支持数千并发连接
- **低延迟**: 基于epoll的非阻塞I/O
- **可扩展**: 支持多服务器实例部署
- **容错性**: 异常处理和资源管理