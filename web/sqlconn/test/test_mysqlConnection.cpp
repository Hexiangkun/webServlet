#include <iostream>
#include "MySqlConnectionPool.h"

using namespace Tiny_muduo;

int main()
{
//    const char *sql =
//            "DROP table if exists person; "
//            "create table person (name varchar(256), age int unsigned, weight double) ENGINE=InnoDB  DEFAULT CHARSET=utf8; "
//            "insert into person(name,age,weight) values('Mary',25,73.8);";
    std::shared_ptr<MySqlConnection> conn = MySqlConnectionPool::getInstance()->getConnection();

//    conn->ExecuteNonQuery(sql);

    const char *name = "Rose";
    int age = 24;
    double weigh = 68.4;
    conn->ExecuteNonQuery("insert into person(name,age,weight) values(?,?,?)", name, age, weigh);

    int minage = 20;
    std::string nm;
    double wg;

    MySqlDataReader *rd = conn->ExecuteReader("select name,weight from person where age > ?", minage);
    while (rd->Read())
    {
        rd->GetValues(nm, wg);
        std::cout << nm << " " << wg << std::endl;
    }
    delete rd;
    return 0;
}