//
// Created by 37496 on 2024/1/30.
//

#ifndef WEBSERVER_LOGFILE_H
#define WEBSERVER_LOGFILE_H

#include <cstdio>
#include <string>
#include <mutex>
#include <memory>
#include "base/Noncopyable.h"


namespace Tiny_muduo
{
    namespace log
    {
        const int FileBufSize = 64*1024;
        template<int SIZE>
        class FileWriter : public Noncopyable
        {
        public:
            explicit FileWriter(const std::string& filename);
            ~FileWriter();

            off_t getWrittenBytes() const;

            void append(const char* line, const size_t len);
            void append(const std::string& line);

            void flush();
        private:
            FILE* _file;
            char _buffer[SIZE];
            off_t _written_bytes;

        };


        class LogFile : public Noncopyable
        {
        public:
            LogFile(const std::string& basename, off_t roll_size);
            ~LogFile();

            void append(const char* line, const size_t len);
            void append(const char* line, const int len);
            void flush();
            void rollFile();

            void setBaseName(const std::string& basename);
        private:

            std::string getLogFileName();

        private:
            std::string _baseName;
            const off_t _roll_size;       //日志文件大小
            int _file_index;         //日志文件id
            std::mutex _mutex;
            std::unique_ptr<FileWriter<FileBufSize>> _file;
        };
    }
}

#endif //WEBSERVER_LOGFILE_H
