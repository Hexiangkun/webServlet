
aux_source_directory(./base HTTP_BASE_LIST)
aux_source_directory(./util HTTP_UTIL_LIST)
aux_source_directory(. HTTP_LIST)

add_executable(httpServer   ./test/test_hhtpserver.cpp
        ${HTTP_BASE_LIST}        ${HTTP_UTIL_LIST}      ${HTTP_LIST})

target_link_libraries(httpServer HConfig HTcp HLog HRedis HMysql
        hiredis mysqlclient ssl crypto uuid)