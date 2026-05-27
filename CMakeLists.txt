cmake_minimum_required(VERSION 3.5)
project(ChatServer)

# C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Debug symbols
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

# Output directory for binaries
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# Header search paths
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/include/server)
include_directories(${PROJECT_SOURCE_DIR}/include/client)
include_directories(${PROJECT_SOURCE_DIR}/json4cpp)

# Subdirectories
add_subdirectory(src)

