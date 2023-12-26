#include <onix/interrupt.h>
#include <onix/syscall.h>
#include <onix/debug.h>
#include <onix/types.h>

// 0号线程
void idle_thread() 
{
    set_interrupt_state(true);
    u32 counter = 0;

    while (true)
    {
        LOGK("idle task... %d\n", counter++);
        asm volatile(
            "sti\n"//开中断
            "hlt\n"//关闭cpu，进入暂停状态，等待外中断的到来
        );

        yield(); // 放弃执行权，调度到其他执行任务，若当前没有就绪的线程就一直执行idle线程
    }
}

// 1号线程
void init_thread()
{
    set_interrupt_state(true);

    while (true)
    {
        LOGK("init task...\n");
        // test();//进行进程阻塞，当前进程被阻塞之后就要调用空闲进程
    }
}