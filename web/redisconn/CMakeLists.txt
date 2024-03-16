
project(redis_test)

aux_source_directory(. REDIS_LIST)

add_library(HRedis STATIC ${REDIS_LIST})

add_executable(redis_test ./test/test_redis.cpp ${REDIS_LIST})

target_link_libraries(redis_test HConfig HLog hiredis)