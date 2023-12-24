#include <onix/debug.h>
#include <onix/assert.h>
#include <onix/types.h>
#include <onix/interrupt.h>
#include <onix/syscall.h>

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

extern void task_yield();

void syscall_init()
{
    for (size_t i = 0; i < SYSCALL_SIZE; i++)
    {
        syscall_table[i] = syscall_default;
    }
    
    syscall_table[SYS_NR_TEST] = sys_test;
    syscall_table[SYS_NR_YIELD] = task_yield;
}