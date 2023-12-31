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

mutex_t mutex;

// 1号线程
void init_thread()
{
    mutex_init(&mutex);
    set_interrupt_state(true);
    u32 counter = 0;

    while (true)
    {
        mutex_lock(&mutex);
        LOGK("init task... %d\n", counter++);
        mutex_unlock(&mutex);
    }
}

void test_thread()
{
    set_interrupt_state(true);
    u32 counter = 0;

    while (true)
    {
        mutex_lock(&mutex);
        LOGK("test task... %d\n", counter++);
        mutex_unlock(&mutex);
    }  
}