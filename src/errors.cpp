#include <cc.h>
#include <stdarg.h>

Logger* Logger::defalut_logger;

Logger::Logger(file& file) : current_file(file) {}

void Logger::trace(bool fatal, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "cc: %s:%d: ", current_file.name, current_file.line);
    fprintf(stderr, "\e[31;1m%s: \e[0m", fatal ? "fatal error" : "error");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);
    if (fatal) exit(1);
}
