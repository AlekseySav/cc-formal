#pragma once

class Logger {
public:
    Logger(file& file);
    void trace(bool fatal, const char* fmt, ...);
public:
    static Logger* defalut_logger;
private:
    file& current_file;
};

#ifndef DEBUG
#define error(fmt, ...) \
    Logger::defalut_logger->trace(false, fmt __VA_OPT__(,) __VA_ARGS__)
#else
#define error(fmt, ...) \
    Logger::defalut_logger->trace(false, "error at %s:%d (%s)", __FILE__, __LINE__, __func__)
#endif

#define fatal(fmt, ...) \
    Logger::defalut_logger->trace(true, fmt __VA_OPT__(,) __VA_ARGS__)
