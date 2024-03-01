//
// Created by 37496 on 2024/1/30.
//

#include <cstring>
#include "LogFile.h"

namespace Tiny_muduo
{
    namespace log
    {
        template<int SIZE>
        FileWriter<SIZE>::FileWriter(const std::string& filename) : _written_bytes(0)
        {
            _file = fopen(filename.c_str(), "a+");
            if(!_file) {
                return;
            }
            ::setbuffer(_file, _buffer, sizeof(_buffer));       //设置文件缓冲区
        }

        template<int SIZE>
        FileWriter<SIZE>::~FileWriter()
        {
            if(_file) {
                fclose(_file);
            }
        }

        template<int SIZE>
        off_t FileWriter<SIZE>::getWrittenBytes() const {
            return _written_bytes;
        }


        template<int SIZE>
        void FileWriter<SIZE>::append(const std::string &line) {
            append(line.c_str(), line.size());
        }

        template<int SIZE>
        void FileWriter<SIZE>::append(const char *line, const size_t len) {
            size_t n = ::fwrite_unlocked(line, 1, len, _file);
            size_t remain = len - n;
            while (remain) {
                size_t x = ::fwrite_unlocked(line+n, 1, remain, _file);
                if(x == 0) {
                    int err = ::ferror(_file);
                    if(err) {
                        fprintf(stderr, "FileWriter::append() failed. %d\n", err);
                        break;
                    }
                }
                n += x;
                remain -= x;
            }
            _written_bytes += len;
        }

        template<int SIZE>
        void FileWriter<SIZE>::flush() {
            ::fflush(_file);
        }


        LogFile::LogFile(const std::string& basename, off_t roll_size)
                : _roll_size(roll_size) , _file_index(0), _baseName(basename) {
            rollFile();
        }
        LogFile::~LogFile() = default;

        std::string LogFile::getLogFileName() {
            std::string filename(_baseName);

            char timebuf[32] = {0};
            struct tm tm;
            time_t now;
            time(&now);
            localtime_r(&now, &tm);
            strftime(timebuf, sizeof (timebuf), "%Y-%m-%d %H:%M:%S.", &tm);
            filename += timebuf;

            char index[8]={0};
            snprintf(index, sizeof(index), "%03d.log", _file_index);
            ++_file_index;
            filename += index;

            return filename;
        }

        void LogFile::setBaseName(const std::string &basename) {
            _baseName = basename;
        }

        void LogFile::append(const char *line, const size_t len) {
            std::unique_lock<std::mutex> lock(_mutex);
            _file->append(line, len);

            if(_file->getWrittenBytes() > _roll_size){
                rollFile();
            }
        }

        void LogFile::append(const char *line, const int len) {
            append(line, static_cast<size_t>(len));
        }

        void LogFile::rollFile() {
            std::string filename = getLogFileName();
            _file.reset(new FileWriter<FileBufSize>(filename));
        }

        void LogFile::flush() {
            std::unique_lock<std::mutex> lock(_mutex);
            _file->flush();
        }
    }
}
