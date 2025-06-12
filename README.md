# ChatServer
A poor chatting server on Linux with TCP Socket
- Linux集群聊天服务器，实现用户注册登陆、收发消息、多人群聊、在线状态管理、离线消息缓存等功能
- 基于 muduo 网络库开发网络模块，提供高并发网络IO服务，实现高效通信，同时解耦网络和业务模块代码
- 使用第三方 JSON 库实现通信数据的序列化和反序列化，传输JSON格式消息
- 使用 Nginx 的 TCP 负载均衡功能，将客户端请求分派到多个服务器上实现集群功能，提高并发处理能力
- 基于 Redis 发布-订阅消息通信模式，实现跨服务器的消息通信；
- 使用 MySQL 存储用户数据和消息
- Windows Subsystem for Linux - Ubuntu 24.04 LTS & Visual Studio Code
