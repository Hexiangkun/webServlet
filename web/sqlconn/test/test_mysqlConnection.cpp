#include <iostream>
#include "sqlconn/MySqlConnectionPool.h"


int main()
{
    const char *sql =
            "DROP table if exists person; "
            "create table `person` (`name` varchar(256), `age` int unsigned, `weight` double) ENGINE=InnoDB  DEFAULT CHARSET=utf8; "
            "insert into person(name,age,weight) values('Mary',25,73.8);";

    std::shared_ptr<SqlConn::MySqlConnection> conn = SqlConn::MySqlConnectionPool::getInstance()->getConnection();

    conn->ExecuteNonQuery(sql);

    const char *name = "Rose";
    const char* pass = "24";

    auto it = conn->ExecuteNonQuery("insert into user(username,passwd) values(?,?)", name, pass);

    int minage = 20;
    std::string nm;
    double wg;

    SqlConn::MySqlDataReader *rd = conn->ExecuteReader("select name,weight from person where age > ?", minage);
    while (rd->Read())
    {
        rd->GetValues(nm, wg);
        std::cout << nm << " " << wg << std::endl;
    }
    delete rd;
    return 0;
}