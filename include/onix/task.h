#ifndef ONIX_TASK_H
#define ONIX_TASK_H

#include <onix/types.h>

typedef u32 target_t(); // 函数地址

typedef struct task_t
{
    /* data */
    u32 *stack;
} task_t;

// 任务寄存器
typedef struct task_frame_t
{
    /* data */
    u32 edi;
    u32 esi;
    u32 ebx;
    u32 ebp;
    void (*eip)(void);
}task_frame_t;

void task_init(); // 任务初始化地址


#endif