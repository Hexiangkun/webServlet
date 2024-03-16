project(echoServer)

aux_source_directory(../base BASE_LIST)

aux_source_directory(./poller POLLER_LIST)
aux_source_directory(./tcp TCP_LIST)
aux_source_directory(./timer TIMER_LIST)

add_library(HTcp STATIC ${BASE_LIST} ${POLLER_LIST} ${TCP_LIST} ${TIMER_LIST})

add_executable(echoServer ./test/echoServer.cpp
        ${BASE_LIST}        ${POLLER_LIST}
        ${TCP_LIST}         ${TIMER_LIST})

target_link_libraries(echoServer HLog ssl crypto)
