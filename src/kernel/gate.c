#include <onix/debug.h>
#include <onix/assert.h>
#include <onix/types.h>
#include <onix/interrupt.h>

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

#define SYSCALL_SIZE 64

handler_t syscall_table[SYSCALL_SIZE]; // 最大有64个系统调用

// 检查系统调用号是否满足要求
void syscall_check(u32 nr)
{
    if (nr >= SYSCALL_SIZE)
    {
        panic("syscall nr error!!!!\n");
    }  
}

static void syscall_default()
{
    panic("syscall not implemented!\n");
}

static u32 sys_test()
{
    LOGK("syscall test.....\n");
    return 255;
}

void syscall_init()
{
    for (size_t i = 0; i < SYSCALL_SIZE; i++)
    {
        syscall_table[i] = syscall_default;
    }
    
    syscall_table[0] = sys_test;
}