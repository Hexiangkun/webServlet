CPP     = g++
RM      = rm

CPPFLAGS = -W -Wall

LDLIBS = -lmysqlclient

all: sample

sample: test_mysqlConnection.cpp ../MySqlConnection.cpp ../MySqlConnection.h ../TmDateTime.cpp ../TmDateTime.h ../MySqlConnectionPool.cpp ../MySqlConnectionPool.h ../../base/Noncopyable.h ../../config/Config.cpp ../../config/Config.h
	$(CPP) $(CPPFLAGS) -o sample test_mysqlConnection.cpp ../MySqlConnectionPool.cpp ../MySqlConnection.cpp ../TmDateTime.cpp ../../config/Config.cpp -I ../../ -I ../ $(LDLIBS)

clean:
	rm -f sample

