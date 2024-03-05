cmake_minimum_required(VERSION 3.22)
project(webserver)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS " -D USE_DEBUG")

include_directories(/usr/local/include/yaml-cpp
                    /usr/local/include/gtest
                    /usr/include/mysql)             # 添加头文件路径
link_directories(/usr/local/lib)                             # 添加库文件路径

include_directories(. ./web ./web/third_party)

aux_source_directory(web/base BASE_LIST)
aux_source_directory(web/config CONFIG_LIST)
aux_source_directory(web/yaml YAML_LIST)
aux_source_directory(web/third_party THIRD_PARTY_LIST)
aux_source_directory(web/log LOG_LIST)

aux_source_directory(web/net/poller POLLER_LIST)
aux_source_directory(web/net/tcp TCP_LIST)
aux_source_directory(web/net/timer TIMER_LIST)

aux_source_directory(web/http/base HTTP_BASE_LIST)
aux_source_directory(web/http/util HTTP_UTIL_LIST)
aux_source_directory(web/http HTTP_LIST)

aux_source_directory(web/sqlconn SQL_CONN_LIST)
aux_source_directory(web/redisconn REDIS_CONN_LIST)
aux_source_directory(web/threadpool THREADPOOL_LIST)

find_package(yaml-cpp REQUIRED)

add_executable(webserver main.cpp
        ${BASE_LIST}
        ${LOG_LIST}
        ${CONFIG_LIST}
        ${YAML_LIST}
        ${TCP_LIST}             ${POLLER_LIST}              ${TIMER_LIST}
        ${HTTP_LIST}            ${HTTP_BASE_LIST}           ${HTTP_UTIL_LIST}
        ${SQL_CONN_LIST}        ${REDIS_CONN_LIST}
        ${THIRD_PARTY_LIST}
        ${THREADPOOL_LIST}
)

set(ALL_LIBS
        yaml-cpp::yaml-cpp
        uuid
        ssl
        crypto
        hiredis
        mysqlclient
)
target_link_libraries(webserver ${ALL_LIBS})