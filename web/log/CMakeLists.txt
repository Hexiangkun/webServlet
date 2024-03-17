aux_source_directory(. LOG_LIST)

add_library(HLog STATIC ${LOG_LIST})

add_executable(logBench ./test/test_log.cpp ${LOG_LIST})

target_link_libraries(logBench HConfig)