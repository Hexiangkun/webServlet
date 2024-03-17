cmake_minimum_required(VERSION 3.22)
project(webserver)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-O2 -DNDEBUG -g -DUSE_LOCKFREEQUEUE")

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)           #可执行文件路径
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)              #库路径

include_directories(/usr/local/include/yaml-cpp
                    /usr/local/include/gtest
                    /usr/include/mysql)                         # 添加库头文件路径


link_directories(/usr/local/lib ${PROJECT_SOURCE_DIR}/lib)      # 添加库文件路径

include_directories(. ./web ./web/third_party)                  #当前头文件路径

add_subdirectory(web/config)
add_subdirectory(web/log)
add_subdirectory(web/redisconn)
add_subdirectory(web/sqlconn)
add_subdirectory(web/yaml)

add_subdirectory(web/net)
add_subdirectory(web/chat)
add_subdirectory(web/http)