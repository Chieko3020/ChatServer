# 项目所需cmake的最低版本
cmake_minimum_required(VERSION 3.0)
#项目的名称
project(ChatServer)

#配置编译选项 对应第2项
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)

#设置可执行文件最终存储的路径 PROJECT_SOURCE_DIR是工程的根目录
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

#配置头文件搜索路径 
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/include/server)
include_directories(${PROJECT_SOURCE_DIR}/include/client)
include_directories(${PROJECT_SOURCE_DIR}/json4cpp)

#加载子目录
add_subdirectory(src)

