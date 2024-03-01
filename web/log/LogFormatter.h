//
// Created by 37496 on 2024/2/9.
//

#ifndef WEBSERVER_LOGFORMATTER_H
#define WEBSERVER_LOGFORMATTER_H

#include <memory>
#include <string>
#include <map>
#include <vector>
#include "LogStream.h"
#include "LogEvent.h"

namespace Tiny_muduo
{
    namespace log
    {
        class FormatItem
        {
        public:
            typedef std::shared_ptr<FormatItem> _ptr;
            virtual void format(LogStream& logStream, const LogEvent& ev) = 0;
        };

        /// @brief 输出字符串
        class PlainFormatItem : public FormatItem
        {
        public:
            explicit PlainFormatItem(const std::string& str) : m_str(str){}
            void format(LogStream& logStream, const LogEvent& ev) override
            {
                logStream << m_str;
            }
        private:
            std::string m_str;
        };

/// @brief 输出日志等级
        class LevelFormatItem : public FormatItem
        {
        public:
            void format(LogStream& logStream, const LogEvent& ev) override
            {
                logStream << LogLevel::toString(ev.getLevel());
            }
        };

/// @brief 输出文件名
        class FileNameFormatItem : public FormatItem
        {
        public:
            void format(LogStream& logStream, const LogEvent& ev) override
            {
                logStream << ev.getFilename();
            }
        };

/// @brief 输出行号
        class LineFormatItem : public FormatItem
        {
        public:
            void format(LogStream& logStream, const LogEvent& ev) override
            {
                logStream << ev.getLine();
            }
        };

/// @brief 输出线程id
        class ThreadIdFormatItem : public FormatItem
        {
        public:
            void format(LogStream& logStream, const LogEvent& ev) override
            {
                logStream << ev.getThreadId();
            }
        };

/// @brief 输出协程id
        class FiberIdFormatItem : public FormatItem
        {
        public:
            void format(LogStream& logStream, const LogEvent& ev) override
            {

            }
        };

/// @brief 输出时间
        class TimeFormatItem : public FormatItem
        {
        public:
            explicit TimeFormatItem(bool showMicroseconds = true):_showMicroseconds(showMicroseconds)
            {
            }
            void format(LogStream& logStream, const LogEvent& ev) override
            {
                logStream << ev.getTime().toFormatString(_showMicroseconds);
            }
        private:
            bool _showMicroseconds;
        };

/// @brief 输出内容
        class ContentFormatItem : public FormatItem
        {
        public:
            void format(LogStream& logStream, const LogEvent& ev) override
            {
                logStream << ev.getContent();
            }
        };

/// @brief 换行
        class NewLineFormatItem : public FormatItem
        {
        public:
            void format(LogStream& logStream, const LogEvent& ev) override
            {
                logStream.append("\n", 2);
            }
        };

/// @brief 百分号%
        class PercentSignFormatItem : public FormatItem
        {
        public:
            void format(LogStream& logStream, const LogEvent& ev) override
            {
                logStream.append("%", 1);
            }
        };

/// @brief 制表符
        class TabFormatItem : public FormatItem
        {
        public:
            void format(LogStream& logStream, const LogEvent& ev) override
            {
                logStream.append("\t", 1);
            }
        };

        thread_local static std::map<char, FormatItem::_ptr> format_item_map{
        #define FN(CH, ITEM_NAME)                   \
            {                                       \
                CH, std::make_shared<ITEM_NAME>()   \
            }
            FN('p', LevelFormatItem),
            FN('f', FileNameFormatItem),
            FN('l', LineFormatItem),
            FN('d', TimeFormatItem),
            FN('F', FiberIdFormatItem),
            FN('t', ThreadIdFormatItem),
            FN('m', ContentFormatItem),
            FN('n', NewLineFormatItem),
            FN('%', PercentSignFormatItem),
            FN('T', TabFormatItem),
        #undef FN
        };

    class LogFormatter
    {
    public:
        using _ptr = std::shared_ptr<LogFormatter>;
    
        LogFormatter(const std::string& pattern);
        ~LogFormatter();
    
        void format(LogStream& logStream, const LogEvent& ev);
    
    private:
        /// @brief 解析m_format_pattern，将对应的字符的格式解析出来放到m_format_items中
        void init();
    
    private:
        std::string m_format_pattern;
        std::vector<FormatItem::_ptr> m_format_items;   //
    };


    }
}




#endif //WEBSERVER_LOGFORMATTER_H
