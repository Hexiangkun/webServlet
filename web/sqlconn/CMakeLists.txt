
project(mysql_test)

aux_source_directory(. MYSQL_LIST)

add_library(HMysql STATIC ${MYSQL_LIST})

add_executable(mysql_test ./test/test_mysqlConnection.cpp ${MYSQL_LIST})

target_link_libraries(mysql_test HConfig mysqlclient)