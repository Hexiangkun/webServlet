本项目的环境配置方案

## CMake
项目使用了CMake进行文件的链接和编译。Ubuntu用户请使用下面的命令安装CMake
```c
sudo apt-get install cmake
```

## Muduo网络库
由于本项目在web/net目录下实现了muduo库，不依赖于boost，读者可以选择跳过。

Muduo网络库的配置较为复杂，也比较容易出错，请仔细的准循着这两篇文章的步骤进行：
- 第一步：安装boost库
  [boost库的安装步骤](https://blog.csdn.net/QIANGWEIYUAN/article/details/88792874)
- 第二步：暗转Muduo库
  [Muduo库的安装步骤](https://blog.csdn.net/QIANGWEIYUAN/article/details/89023980)

如果安装过程中，出现了问题，可以在CSDN上搜索解决办法。一般的错误都能找得到解决方案

## MySQL
本项目需要安装mysql-server以及对应的开发包。ubuntu环境安装mysql-server和mysql开发包，包括mysql头文件和动态库文件，命令如下：
```c
sudo apt-get install mysql-server    =》 安装最新版MySQL服务器
sudo apt-get install libmysqlclient-dev =》 安装开发包
```

安装完成后需要设置MySQL的登录用户和密码。

如果用户名和密码无法修改成功，也可以在CSDN上找到对应的解决方法。

修改在config.json文件中更改对应用户名以及密码

为了能够正确运行项目中的代码，还需导入项目要用到的几张表
导入文件在：[chat.sql](./chat.sql)  



## Nginx

nginx编译安装需要先安装pcre、openssl、zlib等库，这三个库的安装方法[请点击这篇文章](https://blog.csdn.net/somanlee/article/details/69808788)

首先去官网下载最新的Nginx压缩包：[官网下载地址](http://nginx.org/en/download.html)

然后使用tar进行解压
```
tar -zxvf xxx.tar.gz
```
解压之后，进入解压后的文件夹，然后依次执行下面命令
```
./configure --with-stream
make && sudo make install
```
现在，Nginx库就被安装在你的计算机上了

**修改Nginx的配置文件**  
使用命令
```c
cd /usr/local/nginx/conf
```
进入到Nginx的安装位置，修改配置文件nginx.conf
```
sudo vim nginx.conf
```
在原有的文件中，添加如下的代码：
```c
stream {
    upstream MyServer {
        server 127.0.0.1:6000 weight=1 max_fails=3 fail_timeout=30s;
        server 127.0.0.1:6002 weight=1 max_fails=3 fail_timeout=30s;
     }
    server {
     proxy_connect_timeout 1s;
     # proxy_timeout 3s;    #不要配置此行，否则客户端会运行3秒就会断开连接
     listen 8000;
     proxy_pass MyServer;
     tcp_nodelay on;
     }
}
```
通过这样的配置后，可以让Nginx帮我们代理位于6000和6002端口的chatserver服务器程序。两个服务器程序的权重相等。我们通过8000端口访问Nginx服务器，就能正常访问chatserver

## Redis
```
sudo apt-get install redis-server   # ubuntu命令安装redis服务
```

redis支持多种不同的客户端编程语言，C++对应的则是hiredis。下面是安装hiredis的步骤：
```c
1. git clone https://github.com/redis/hiredis      从github上下载hiredis客户端，进行源码编译安装
2. cd hiredis
3. make
4. sudo make install
5. sudo ldconfig /usr/local/lib
```

所有的依赖环境，都在[src/server/CMakeLists.txt](../src/server/CMakeLists.txt)的这一行命令中得以体现：
```c
target_link_libraries(server muduo_net muduo_base mysqlclient hiredis pthread)
```

## Yaml
```c
git clone https://github.com/jbeder/yaml-cpp.git
cd yaml-cpp && mkdir build && cd build
cmake -DYAML_BUILD_SHARED_LIBS=on .. #共享库；可以设置未off，则为静态库
sudo make && sudo make install
sudo ldconfig
```

## 其他依赖
```
sudo apt-get install libssl-dev
sudo apt-get install libcrypto++-dev libcrypto++-doc libcrypto++-utils
sudo apt-get install uuid-dev
```