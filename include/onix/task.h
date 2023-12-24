#ifndef ONIX_TASK_H
#define ONIX_TASK_H

#include <onix/types.h>
#include <onix/bitmap.h>
#include <onix/list.h>

#define KERNEL_USER 0
#define NORMAL_USER 1

#define TASK_NAME_LEN 16



typedef u32 target_t(); // 函数地址

typedef enum task_state_t
{
    TASK_INIT,     // 初始化
    TASK_RUNNING,  // 执行
    TASK_READY,    // 就绪
    TASK_BLOCKED,  // 阻塞
    TASK_SLEEPING, // 睡眠 阻塞的细分
    TASK_WAITING,  // 等待 阻塞的细分
    TASK_DIED,     // 死亡
} task_state_t;

// PCB 任务描述符
typedef struct task_t
{
    u32 *stack;             // 内核栈
    list_node_t node;       // 任务阻塞节点 
    task_state_t state;     // 任务状态
    u32 priority;           // 任务优先级
    u32 ticks;              // 剩余时间片
    u32 jiffies;            // 上次执行时全局时间片
    char name[TASK_NAME_LEN];// 任务名，防止发生溢出
    u32 uid;                // 用户ID
    u32 pde;                // 页目录物理地址
    struct bitmap_t *vmap;         // 进程虚拟内存位图
    u32 magic;              // 内核魔数，用于检查栈溢出
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

task_t *running_task(); // 获取运行任务的栈信息
void schedule(); // 任务调度接口

void task_yield(); // 主动让出cpu

// 任务枷锁与解锁过程
void task_block(task_t *task, list_t *blist, task_state_t state);
void task_unblock(task_t *task);


#endif