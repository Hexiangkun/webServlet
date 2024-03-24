//
// Created by 37496 on 2024/3/23.
//
#include "net/tcp/TcpServer.h"
#include "net/tcp/EventLoop.h"
#include "net/tcp/TcpConnection.h"
#include "log/Log.h"

const int kBufSize = 64*1024;
const char* g_file = nullptr;
typedef std::shared_ptr<FILE> FilePtr;

void onHighWaterMark(const Tiny_muduo::net::TcpConnectionPtr& conn, size_t len)
{
    LOG_INFO << "HighWaterMark " << len;
}

void onConnection(const Tiny_muduo::net::TcpConnectionPtr& conn) {
    LOG_INFO << "FileServer - " << conn->peerAddress().toString() << " -> "
             << conn->localAddress().toString() << " is "
             << (conn->connected() ? "UP" : "DOWN");
    if(conn->connected()) {
        LOG_INFO << "FileServer - Sending file " << g_file
                 << " to " << conn->peerAddress().toString();

        conn->setHighWaterMarkCallback(onHighWaterMark, kBufSize+1);

        FILE * fp = ::fopen(g_file, "rb");
        if(fp) {
            FilePtr ctx(fp, ::fclose);
            conn->setAnyContext(ctx);
            char buf[kBufSize];
            size_t nread = ::fread(buf, 1, sizeof buf, fp);
            conn->send(buf, nread);
        }
        else
        {
            conn->shutdown();
            LOG_INFO << "FileServer - no such file";
        }
    }
}


void onWriteComplete(const Tiny_muduo::net::TcpConnectionPtr& conn)
{
    const FilePtr& fp = std::any_cast<const FilePtr&>(conn->getAnyContext());
    char buf[kBufSize];
    size_t nread = ::fread(buf, 1, sizeof buf, fp.get());
    if (nread > 0)
    {
        conn->send(buf, nread);
    }
    else
    {
        conn->shutdown();
        LOG_INFO << "FileServer - done";
    }
}

int main(int argc, char* argv[])
{
    LOG_INFO << "pid = " << getpid();
    if(argc > 1) {
        g_file = argv[1];

        Tiny_muduo::net::EventLoop loop;
        Tiny_muduo::net::InetAddress listenAddr(2021);
        Tiny_muduo::net::TcpServer server(&loop, listenAddr, "FileServer");
        server.setConnectionCallback(onConnection);
        server.setWriteCompleteCallback(onWriteComplete);
        server.start();
        loop.loop();
    }
    else
    {
        fprintf(stderr, "Usage: %s file_for_downloading\n", argv[0]);
    }
}