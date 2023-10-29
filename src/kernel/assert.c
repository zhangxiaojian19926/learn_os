#include <onix/console.h>
#include <onix/printk.h>
#include <onix/stdarg.h>
#include <onix/stdio.h>

static u8 buf[1024];

// 强制阻塞
static void spin(char *name)
{
    printk("spinnig in %s ...\n", name);
    while (true)
    {
        ;
    }
}

void assertion_failure(char *exp, char *file, char *base, int line)
{
    printk(
        "\n--> assert(%s) failed!!!\n"
        "--> file: %s \n"
        "--> base: %s \n"
        "--> line: %d \n", 
        exp, file, base, line);

    spin("assertion_failue()");

    // 不可能走到着，否则汇编报错
    asm volatile("ud2");
}

// 系统重大bug报错
void panic(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int i = vsprintf(buf, fmt, args);
    va_end(args);

    printk("!!! panic !!!\n -->%s \n", buf);
    spin("panic !!!!");

    // 不可能走到着，否则汇编报错
    asm volatile("ud2");
}