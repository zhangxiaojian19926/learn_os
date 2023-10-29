#ifndef ONIX_STDIO_H
#define ONIX_STDIO_H

#include <onix/stdarg.h>

int vsprintf(char *buf, const char *fmt, va_list args);// 内核中的格式化输出
int sprintf(char *buf, const char *fmt, ...);//格式化输出到buf指针中

#endif