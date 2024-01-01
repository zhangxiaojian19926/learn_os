#ifndef ONIX_MUTEX_H
#define ONIX_MUTEX_H 

#include <onix/types.h>
#include <onix/task.h>
#include <onix/list.h>

// 信号量
typedef struct mutex_t
{
    bool value;    // 信号量
    list_t waiters; // 等待队列
} mutex_t;

void mutex_init(mutex_t *mutex);   // 初始化信号量
void mutex_lock(mutex_t *mutex);   // 尝试持有互斥量
void mutex_unlock(mutex_t *mutex); // 释放信号量

// 自旋锁
typedef struct spinlock_t
{
    task_t *holder; // 持有者
    mutex_t mutex;  // 互斥量
    u32     repeat; // 重入次数
}spinlock_t;

void spin_init(spinlock_t *lock);   // 自旋锁初始化
void spin_lock(spinlock_t *lock);   // 加锁
void spin_unlock(spinlock_t *lock); // 解锁

#endif