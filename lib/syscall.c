#include <onix/syscall.h>

// 系统层与应用层之间的桥梁
static _inline u32 _syscall0(u32 nr)
{
    u32 ret;
    asm volatile(
        "int $0x80\n"
        :"=a"(ret)
        :"a"(nr)
    );

    return ret;
}

static _inline u32 _syscall1(u32 nr, u32 arg)
{
    u32 ret;
    asm volatile(
        "int $0x80\n"
        :"=a"(ret)
        :"a"(nr), "b"(arg)
    );// 第一个参数放入到eax中，arg放入到ebx中

    return ret;
}

u32 test()
{
    return _syscall0(SYS_NR_TEST);
}

void yield()  // 系统层接口
{
    _syscall0(SYS_NR_YIELD);
}

void sleep(u32 ms)
{
    _syscall1(SYS_NR_SLEEP, ms);
}