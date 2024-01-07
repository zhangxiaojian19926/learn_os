#include <onix/debug.h>
#include <onix/assert.h>
#include <onix/types.h>
#include <onix/interrupt.h>
#include <onix/syscall.h>
#include <onix/task.h>
#include <onix/console.h>

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

task_t *task = NULL;

static u32 sys_test()
{
    // LOGK("syscall test.....\n");
    if (!task)
    {
        /* code */
        task = running_task();
        // LOGK("block task 0x%p \n", task);
        task_block(task, NULL, TASK_BLOCKED);
    }
    else
    {
        task_unblock(task);
        // LOGK("unblock task 0x%p \n", task);
        task = NULL;
    }
    
    return 255;
}

int32 sys_write(fd_t fd, char *buf, u32 len)
{
    if (stdout == fd || stderr == fd)
    {
        return console_write(buf, len);
    }
    
    panic("write !!!!\n");
    return 0;
}

void syscall_init()
{
    for (size_t i = 0; i < SYSCALL_SIZE; i++)
    {
        syscall_table[i] = syscall_default;
    }
    
    syscall_table[SYS_NR_TEST] = sys_test;
    syscall_table[SYS_NR_SLEEP] = task_sleep;
    syscall_table[SYS_NR_YIELD] = task_yield;
    syscall_table[SYS_NR_WRITE] = sys_write;
}