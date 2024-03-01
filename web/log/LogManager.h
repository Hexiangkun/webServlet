////
//// Created by 37496 on 2024/2/9.
////
//
//#ifndef WEBSERVER_LOGMANAGER_H
//#define WEBSERVER_LOGMANAGER_H
//
//#include <memory>
//#include <map>
//#include "Logger.h"
//#include "mutex.h"
//#include "singleinstance.h"
//
//namespace Tiny_muduo
//{
//    namespace log
//    {
//        class LoggerManager
//        {
//
//        public:
//            using _ptr = std::shared_ptr<LoggerManager>;
//
//            LoggerManager(/* args */);
//            ~LoggerManager();
//
//            Logger::_ptr getLogger(const std::string& name);
//            Logger::_ptr getGlobalLogger();
//            void init(std::vector<LoggerConfig> config_logs = {});
//        private:
//
//            void ensureGlobalLoggerExists();
//
//
//        private:
//            std::map<std::string, Logger::_ptr> m_logger_map;
//            MutexLock m_mutex;
//        };
//
//        typedef SingleInstancePtr<LoggerManager> LoggerManager_Ptr;
//
//
//        LoggerManager::LoggerManager(/* args */)
//        {
//            init();
//        }
//
//        LoggerManager::~LoggerManager()
//        {
//        }
//
//        void LoggerManager::init(std::vector<LoggerConfig> configs)
//        {
//            ScopedLock lock(&m_mutex);
//            auto config_vec = Config::lookUp<std::vector<LoggerConfig>>("logs");
//
//            for(const auto& config : configs)
//            {
//                m_logger_map.erase(config.name);
//                auto logger = std::make_shared<Logger>(config.name, config.level, config.formatter);
//                for(const auto& config_appender : config.appenders)
//                {
//                    LogAppender::_ptr appender;
//                    switch (config_appender.type)
//                    {
//                        case LogAppenderConfig::STDOUT:
//                            appender = std::make_shared<StdoutLogAppender>(config_appender.level);
//                            break;
//                        case LogAppenderConfig::FILE:
//                            appender = std::make_shared<FileLogAppender>(config_appender.filepath, config_appender.level);
//                            break;
//                        case LogAppenderConfig::ASYNCFILE:
//                            appender = std::make_shared<AsyncFileLogAppender>(config_appender.filepath, config_appender.level);
//                            break;
//                        default:
//                            std::cerr << "__LoggerManager::init exception invalid appender' values, appender.type=" << config_appender.type << std::endl;
//                            break;
//                    }
//
//                    if(!config_appender.formatter.empty()) {
//                        appender->setFormatter(std::make_shared<LogFormatter>(config_appender.formatter));
//                    }
//                    logger->addAppender(std::move(appender));
//                }
//                m_logger_map.insert(std::make_pair(config.name, std::move(logger)));
//            }
//
//            ensureGlobalLoggerExists();
//
//        }
//
//        void LoggerManager::ensureGlobalLoggerExists()
//        {
//            auto iter = m_logger_map.find("global");
//            if(iter == m_logger_map.end()) {
//                auto global_logger = std::make_shared<Logger>();
//                global_logger->setName("global");
//                global_logger->addAppender(std::make_shared<StdoutLogAppender>());
//                m_logger_map.insert(std::make_pair("global", std::move(global_logger)));
//            }
//            else if(!iter->second) {
//                iter->second = std::make_shared<Logger>();
//                iter->second->setName("global");
//                iter->second->addAppender(std::make_shared<StdoutLogAppender>());
//            }
//        }
//
//        Logger::_ptr LoggerManager::getLogger(const std::string& name)
//        {
//            ScopedLock lock(&m_mutex);
//            if(m_logger_map.count(name)) {
//                return m_logger_map[name];
//            }
//            else {
//                return m_logger_map["global"];
//            }
//        }
//
//        Logger::_ptr LoggerManager::getGlobalLogger()
//        {
//            if(!m_logger_map.count("global")) {
//                ensureGlobalLoggerExists();
//            }
//            return m_logger_map["global"];
//        }
//
//
//    }
//}
//
//
//#endif //WEBSERVER_LOGMANAGER_H
