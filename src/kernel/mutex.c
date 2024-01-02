#include <onix/mutex.h>
#include <onix/task.h>
#include <onix/interrupt.h>
#include <onix/assert.h>

void mutex_init(mutex_t *mutex)
{
    mutex->value = false; // 初始化时没有被人占有
    list_init(&mutex->waiters);// 初始化等待队列
}

// 尝试持有互斥量
void mutex_lock(mutex_t *mutex)
{
    // 关闭中断，保证操作原子性
    bool intr = interrupt_disable();

    task_t *current = running_task();
    while (true == mutex->value)
    {
        // 若value为true，代表有人持有锁，则当前强锁线程需要放入等待队列
        task_block(current, &mutex->waiters, TASK_BLOCKED);
    }

    // 出了while时，一定是无人持有的
    assert(mutex->value == false);

    // 当前线程持有锁
    mutex->value++;
    assert(mutex->value == true);// ++ 之后的值必须要等于true

    // 恢复之前中断标志
    set_interrupt_state(intr);    
}

// 释放信号量
void mutex_unlock(mutex_t *mutex)
{
    bool intr = interrupt_disable();

    // 当前线程已经持有锁，以下是释放锁流程
    assert(mutex->value == true);

    // 取消持有
    mutex->value--;
    assert(mutex->value == false);

    // 如果等待队列是空，则不需要释放
    // 若不是，则需要当前线程释放锁，提供给后面的线程
    // 采用的是先进先出的策略，从waiters队列的队尾进行查找，
    // 找到最先进入队列的那个任务，它可以拿锁
    if (!list_empty(&mutex->waiters))
    {
        task_t *task = element_entry(task_t, node, mutex->waiters.tail.prev);
        assert(task->magic == ONIX_MAGIC); //魔数校验

        task_unblock(task);
        // 当前线程必须让出cpu，防止一直被当前线程掉哟个
        task_yield();
    }
    
    // 恢复之前中断标志
    set_interrupt_state(intr); 
}

// 自旋锁初始化
void lock_init(lock_t *lock)
{
    lock->holder = NULL;
    lock->repeat = 0;
    mutex_init(&lock->mutex);
}

// 尝试持有锁
void lock_acquire(lock_t *lock)
{
    task_t *current = running_task();

    // 若持有者不是当前线程，则进入，清除上次的持有者，对信号量进行加锁
    if (lock->holder != current)
    {
        mutex_lock(&lock->mutex);
        lock->holder = current;
        assert(lock->repeat == 0);// 不等于assert
        lock->repeat = 1; 
    }
    else
    {
        lock->repeat++;
    }  
}

// 解锁
void lock_release(lock_t *lock)
{
    task_t *current = running_task();
    assert(lock->holder == current);

    // 若自旋锁被当前进程重复占用时，这个时候则需要将repeat只为0
    if (lock->repeat > 1)
    {
        lock->repeat--;
        return;
    }

    // 只有当repeat减为1才代表当前进程释放一次就够了
    assert(lock->repeat == 1);

    lock->holder = NULL;
    lock->repeat = 0;
    mutex_unlock(&lock->mutex); 
}