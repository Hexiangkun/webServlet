//
// Created by 37496 on 2024/2/9.
//

#include "LogFormatter.h"


namespace Tiny_muduo
{
    namespace log
    {
        LogFormatter::LogFormatter(const std::string& pattern):m_format_pattern(pattern)
        {
            init();
        }

        LogFormatter::~LogFormatter()
        {

        }

        void LogFormatter::format(LogStream& logStream, const LogEvent& ev)
        {
            for(const auto& item : m_format_items) {
                item->format(logStream, ev);
            }
        }


        void LogFormatter::init()
        {
            enum PRASE_STATUS
            {
                SCAN_STATUS,    //扫描到普通字符
                CREATE_STATUS   //扫描到%,处理占位符
            };
            PRASE_STATUS status = SCAN_STATUS;
            ssize_t str_begin = 0;
            ssize_t str_end = 0;
            //日志格式"[%d] [%p] [%f]%T%m%n"
            for(size_t i = 0; i < m_format_pattern.size(); i++)
            {
                switch (status)
                {
                    case SCAN_STATUS:
                        str_begin = i;
                        for(str_end = i ; str_end < m_format_pattern.size(); str_end++) {
                            if(m_format_pattern[str_end] == '%') {
                                status = CREATE_STATUS;
                                break;
                            }
                        }
                        i = str_end;
                        m_format_items.emplace_back(std::make_shared<PlainFormatItem>(m_format_pattern.substr(str_begin, str_end-str_begin)));
                        break;
                    case CREATE_STATUS:
                        assert(!format_item_map.empty() && "format_item_map don't init");
                        auto iter = format_item_map.find(m_format_pattern[i]);
                        if(iter == format_item_map.end()) {
                            m_format_items.emplace_back(std::make_shared<PlainFormatItem>("<error format>"));
                        }
                        else {
                            m_format_items.emplace_back(iter->second);
                        }
                        status = SCAN_STATUS;
                        break;
                }
            }
        }
    }
}