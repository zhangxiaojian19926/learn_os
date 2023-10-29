#include <onix/stdarg.h>
#include <onix/console.h>
#include <onix/stdio.h>

static char buf[1024];// printk 的缓存

int printk(const char *fmt, ...)
{
    va_list args;
    int i;

    va_start(args, fmt); // 可变参数开始

    i = vsprintf(buf, fmt, args);// 格式化输入参数

    va_end(args); // 可变参数结束

    console_write(buf, i); // 写到屏幕上

    return i;
}