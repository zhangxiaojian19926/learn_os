#include <onix/interrupt.h>
#include <onix/syscall.h>
#include <onix/debug.h>
#include <onix/types.h>
#include <onix/mutex.h>

// 0号线程
void idle_thread() 
{
    set_interrupt_state(true);
    u32 counter = 0;

    while (true)
    {
        // LOGK("idle task... %d\n", counter++);
        asm volatile(
            "sti\n"//开中断
            "hlt\n"//关闭cpu，进入暂停状态，等待外中断的到来
        );

        yield(); // 放弃执行权，调度到其他执行任务，若当前没有就绪的线程就一直执行idle线程
    }
}

lock_t lock;

// 1号线程
void init_thread()
{
    lock_init(&lock);
    set_interrupt_state(true);
    u32 counter = 0;

    while (true)
    {
        lock_acquire(&lock);
        LOGK("init task... %d\n", counter++);
        lock_release(&lock);
    }
}

void test_thread()
{
    set_interrupt_state(true);
    u32 counter = 0;

    while (true)
    {
        lock_acquire(&lock);
        LOGK("test task... %d\n", counter++);
        lock_release(&lock);
    }  
}