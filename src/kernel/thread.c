#include <onix/interrupt.h>
#include <onix/syscall.h>
#include <onix/debug.h>
#include <onix/types.h>
#include <onix/mutex.h>
#include <onix/printk.h>

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

extern u32 keyboard_read(char *buf, u32 count);

// 1号线程
void init_thread()
{
    set_interrupt_state(true);
    u32 counter = 0;

    char ch;
    while (true)
    {
        bool intr = interrupt_disable();
        keyboard_read(&ch, 1);
        printk("%c", ch); // 读取字符回显到屏幕上
        set_interrupt_state(intr);
    }
}

void test_thread()
{
    set_interrupt_state(true);
    u32 counter = 0;

    while (true)
    {
        sleep(709);
    }  
}