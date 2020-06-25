/**
 * @Author: choubin
 * @Date: 2020-06-14 22:37:38
 * @LastEditors: choubin
 * @LastEditTime: 2020-06-14 22:38:01
 * @Description: 日志模块实现
 */

#ifndef __GEDUO_LOG_H__
#define __GEDUO_LOG_H__

#include <stdarg.h>
#include <stdint.h>

#include <fstream>
#include <list>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "singleton.h"
#include "thread.h"
#include "util.h"

/**
 * @brief 使用流式方式将日志级别level的日志写入到logger
 */
#define GEDUO_LOG_LEVEL(logger, level)                                                \
    if (logger->getLevel() <= level)                                                  \
    geduo::LogEventWrap(geduo::LogEvent::ptr(new geduo::LogEvent(logger, level,       \
                            __FILE__, __LINE__, 0, geduo::GetThreadId(),              \
                            geduo::GetFiberId(), time(0), geduo::Thread::GetName()))) \
        .getSS()

/**
 * @brief 使用流式方式将日志级别debug的日志写入到logger
 */
#define GEDUO_LOG_DEBUG(logger) GEDUO_LOG_LEVEL(logger, geduo::LogLevel::DEBUG)

/**
 * @brief 使用流式方式将日志级别info的日志写入到logger
 */
#define GEDUO_LOG_INFO(logger) GEDUO_LOG_LEVEL(logger, geduo::LogLevel::INFO)

/**
 * @brief 使用流式方式将日志级别warn的日志写入到logger
 */
#define GEDUO_LOG_WARN(logger) GEDUO_LOG_LEVEL(logger, geduo::LogLevel::WARN)

/**
 * @brief 使用流式方式将日志级别error的日志写入到logger
 */
#define GEDUO_LOG_ERROR(logger) GEDUO_LOG_LEVEL(logger, geduo::LogLevel::ERROR)

/**
 * @brief 使用流式方式将日志级别fatal的日志写入到logger
 */
#define GEDUO_LOG_FATAL(logger) GEDUO_LOG_LEVEL(logger, geduo::LogLevel::FATAL)

/**
 * @brief 使用格式化方式将日志级别level的日志写入到logger
 */
#define GEDUO_LOG_FMT_LEVEL(logger, level, fmt, ...)                                  \
    if (logger->getLevel() <= level)                                                  \
    geduo::LogEventWrap(geduo::LogEvent::ptr(new geduo::LogEvent(logger, level,       \
                            __FILE__, __LINE__, 0, geduo::GetThreadId(),              \
                            geduo::GetFiberId(), time(0), geduo::Thread::GetName()))) \
        .getEvent()                                                                   \
        ->format(fmt, __VA_ARGS__)

/**
 * @brief 使用格式化方式将日志级别debug的日志写入到logger
 */
#define GEDUO_LOG_FMT_DEBUG(logger, fmt, ...) GEDUO_LOG_FMT_LEVEL(logger, geduo::LogLevel::DEBUG, fmt, __VA_ARGS__)

/**
 * @brief 使用格式化方式将日志级别info的日志写入到logger
 */
#define GEDUO_LOG_FMT_INFO(logger, fmt, ...) GEDUO_LOG_FMT_LEVEL(logger, geduo::LogLevel::INFO, fmt, __VA_ARGS__)

/**
 * @brief 使用格式化方式将日志级别warn的日志写入到logger
 */
#define GEDUO_LOG_FMT_WARN(logger, fmt, ...) GEDUO_LOG_FMT_LEVEL(logger, geduo::LogLevel::WARN, fmt, __VA_ARGS__)

/**
 * @brief 使用格式化方式将日志级别error的日志写入到logger
 */
#define GEDUO_LOG_FMT_ERROR(logger, fmt, ...) GEDUO_LOG_FMT_LEVEL(logger, geduo::LogLevel::ERROR, fmt, __VA_ARGS__)

/**
 * @brief 使用格式化方式将日志级别fatal的日志写入到logger
 */
#define GEDUO_LOG_FMT_FATAL(logger, fmt, ...) GEDUO_LOG_FMT_LEVEL(logger, geduo::LogLevel::FATAL, fmt, __VA_ARGS__)

/**
 * @brief 获取主日志器
 */
#define GEDUO_LOG_ROOT() geduo::LoggerMgr::GetInstance()->getRoot()

/**
 * @brief 获取name的日志器
 */
#define GEDUO_LOG_NAME(name) geduo::LoggerMgr::GetInstance()->getLogger(name)

namespace geduo {
class Logger;
class LoggerManager;

class LogLevel {
public:
    /// @brief 日志级别的枚举类型
    enum Level {
        UNKNOW = 0,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL
    };

    /// @brief 将日志级别转为本文
    static const char* ToString(LogLevel::Level level);

    /// @brief 将文件转换为日志级别
    static LogLevel::Level FromString(const std::string& str);
};

/// @brief 日志事件
class LogEvent {
public:
    typedef std::shared_ptr<LogEvent> ptr;
    /**
   * @brief 构造函数
   * @param[in] logger 日志器
   * @param[in] level 日志级别
   * @param[in] file 文件名
   * @param[in] line 文件行号
   * @param[in] elapse 程序启动依赖的耗时(毫秒)
   * @param[in] thread_id 线程id
   * @param[in] fiber_id 协程id
   * @param[in] time 日志事件(秒)
   * @param[in] thread_name 线程名称
   */
    LogEvent(std::shared_ptr<Logger> logger,
        LogLevel::Level level,
        const char* file,
        int32_t line,
        uint32_t elapse,
        uint32_t thread_id,
        uint32_t fiber_id,
        uint64_t time,
        const std::string& thread_name);

    /// @brief 返回文件名
    const char* getFile() const { return m_file; }

    /// @brief 返回行号
    int32_t getLine() const { return m_line; }

    /// @brief 返回耗时
    uint32_t getElapse() const { return m_elapse; }

    /// @brief 返回线程ID
    uint32_t getThreadId() const { return m_threadId; }

    /// @brief 返回协程ID
    uint32_t getFiberId() const { return m_fiberId; }

    /// @brief 返回时间
    uint64_t getTime() const { return m_time; }

    /// @brief 返回线程名称
    const std::string& getThreadName() const { return m_threadName; }

    /// @brief 返回日志内容
    std::string getContent() const { return m_ss.str(); }

    ///@brief 返回日志器
    std::shared_ptr<Logger> getLogger() const { return m_logger; }

    /// @brief 返回日志级别
    LogLevel::Level getLevel() const { return m_level; }

    /// @brief 返回日志内容字符串流
    std::stringstream& getSS() { return m_ss; }

    /// @brief 格式化写入日志内容
    void format(const char* fmt, ...);

    /// @brief 格式化写入日志内容
    void format(const char* fmt, va_list al);

private:
    /// 文件名
    const char* m_file = nullptr;
    /// 行号
    int32_t m_line = 0;
    /// 程序启动开始到现在的毫秒数
    uint32_t m_elapse = 0;
    /// 线程ID
    uint32_t m_threadId = 0;
    /// 协程ID
    uint32_t m_fiberId = 0;
    /// 时间戳
    uint64_t m_time = 0;
    /// 线程名称
    std::string m_threadName;
    /// 日志内容流
    std::stringstream m_ss;
    /// 日志器
    std::shared_ptr<Logger> m_logger;
    /// 日志等级
    LogLevel::Level m_level;
};

/// @brief 日志事件包装器
class LogEventWrap {
public:
    /**
     * @brief 构造函数
     * @param[in] e 日志事件
     */
    LogEventWrap(LogEvent::ptr e);

    ~LogEventWrap();

    /// @brief 获取日志事件
    LogEvent::ptr getEvent() const { return m_event; }

    /// @brief 获取日志内容流
    std::stringstream& getSS();

private:
    /// @brief 日志事件
    LogEvent::ptr m_event;
};

/// @brief 日志格式化
class LogFormatter {
public:
    typedef std::shared_ptr<LogFormatter> ptr;
    /**
     * @brief 构造函数
     * @param[in] pattern 格式模板
     * @details 
     *  %m 消息
     *  %p 日志级别
     *  %r 累计毫秒数
     *  %c 日志名称
     *  %t 线程id
     *  %n 换行
     *  %d 时间
     *  %f 文件名
     *  %l 行号
     *  %T 制表符
     *  %F 协程id
     *  %N 线程名称
     *
     *  默认格式 "%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"
     */
    LogFormatter(const std::string& pattern);

    /**
     * @brief 返回格式化日志文本
     * @param[in] logger 日志器
     * @param[in] level 日志级别
     * @param[in] event 日志事件
     */
    std::string format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event);
    std::ostream& format(std::ostream& ofs, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event);

public:
    /**
     * @brief 日志内容项格式化
     */
    class FormatItem {
    public:
        typedef std::shared_ptr<FormatItem> ptr;
        /**
         * @brief 析构函数
         */
        virtual ~FormatItem() {}
        /**
         * @brief 格式化日志到流
         * @param[in, out] os 日志输出流
         * @param[in] logger 日志器
         * @param[in] level 日志等级
         * @param[in] event 日志事件
         */
        virtual void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;
    };

    /// @brief 初始化,解析日志模板
    void init();

    /// @brief 是否有错误
    bool isError() const { return m_error; }

    /// @brief 返回日志模板
    const std::string getPattern() const { return m_pattern; }

private:
    /// 日志格式模板
    std::string m_pattern;
    /// 日志格式解析后格式
    std::vector<FormatItem::ptr> m_items;
    /// 是否有错误
    bool m_error = false;
};

/// @brief 日志输出目标
class LogAppender {
    friend class Logger;

public:
    typedef std::shared_ptr<LogAppender> ptr;
    typedef Spinlock MutexType;

    virtual ~LogAppender() {}

    /**
     * @brief 写入日志
     * @param[in] logger 日志器
     * @param[in] level 日志级别
     * @param[in] event 日志事件
     */
    virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;

    /// @brief 将日志输出目标的配置转成YAML String
    virtual std::string toYamlString() = 0;

    /// @brief 更改日志格式器
    void setFormatter(LogFormatter::ptr val);

    /// @brief 获取日志格式器
    LogFormatter::ptr getFormatter();

    /// @brief 获取日志级别
    LogLevel::Level getLevel() const { return m_level; }

    /// @brief 设置日志级别
    void setLevel(LogLevel::Level val) { m_level = val; }

protected:
    /// 日志级别
    LogLevel::Level m_level = LogLevel::DEBUG;
    /// 是否有自己的日志格式器
    bool m_hasFormatter = false;
    /// Mutex
    MutexType m_mutex;
    /// 日志格式器
    LogFormatter::ptr m_formatter;
};

/// @brief 日志器
class Logger : public std::enable_shared_from_this<Logger> {
    friend class LoggerManager;

public:
    typedef std::shared_ptr<Logger> ptr;
    typedef Spinlock MutexType;

    Logger(const std::string& name = "root");

    /**
     * @brief 写日志
     * @param[in] level 日志级别
     * @param[in] event 日志事件
     */
    void log(LogLevel::Level level, LogEvent::ptr event);

    /// @brief 写debug级别日志
    void debug(LogEvent::ptr event);

    /// @brief 写info级别日志
    void info(LogEvent::ptr event);

    /// @brief 写warn级别日志
    void warn(LogEvent::ptr event);

    /// @brief 写error级别日志
    void error(LogEvent::ptr event);

    /// @brief 写fatal级别日志
    void fatal(LogEvent::ptr event);

    /// @brief 添加日志目标
    void addAppender(LogAppender::ptr appender);

    /// @brief 删除日志目标
    void delAppender(LogAppender::ptr appender);

    /// @brief 清空日志目标
    void clearAppenders();

    /// @brief 返回日志级别
    LogLevel::Level getLevel() const { return m_level; }

    /// @brief 设置日志级别
    void setLevel(LogLevel::Level val) { m_level = val; }

    /// @brief 返回日志名称
    const std::string& getName() const { return m_name; }

    /// @brief 设置日志格式器
    void setFormatter(LogFormatter::ptr val);

    /// @brief 设置日志格式模板
    void setFormatter(const std::string& val);

    /// @brief 获取日志格式器
    LogFormatter::ptr getFormatter();

    /// @brief 将日志器的配置转成YAML String
    std::string toYamlString();

private:
    /// 日志名称
    std::string m_name;
    /// 日志级别
    LogLevel::Level m_level;
    /// Mutex
    MutexType m_mutex;
    /// 日志目标集合
    std::list<LogAppender::ptr> m_appenders;
    /// 日志格式器
    LogFormatter::ptr m_formatter;
    /// 主日志器
    Logger::ptr m_root;
};

/// @brief 输出到控制台的Appender
class StdoutLogAppender : public LogAppender {
public:
    typedef std::shared_ptr<StdoutLogAppender> ptr;
    void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override;
    std::string toYamlString() override;
};

/// @brief 输出到文件的Appender
class FileLogAppender : public LogAppender {
public:
    typedef std::shared_ptr<FileLogAppender> ptr;
    FileLogAppender(const std::string& filename);
    void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override;
    std::string toYamlString() override;

    /// @brief 重新打开日志文件
    bool reopen();

private:
    /// 文件路径
    std::string m_filename;
    /// 文件流
    std::ofstream m_filestream;
    /// 上次重新打开时间
    uint64_t m_lastTime = 0;
};

/// @brief 日志器管理类
class LoggerManager {
public:
    typedef Spinlock MutexType;

    LoggerManager();

    /// @brief 通过日志器名称获取日志器
    Logger::ptr getLogger(const std::string& name);

    void init();

    /// @brief 返回主日志器
    Logger::ptr getRoot() const { return m_root; }

    /// @brief 将所有的日志器配置转成YAML String
    std::string toYamlString();

private:
    /// Mutex
    MutexType m_mutex;
    /// 日志器容器
    std::map<std::string, Logger::ptr> m_loggers;
    /// 主日志器
    Logger::ptr m_root;
};

/// 日志器管理类单例模式
typedef geduo::Singleton<LoggerManager> LoggerMgr;

} // namespace geduo

#endif