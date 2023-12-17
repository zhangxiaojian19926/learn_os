#include <onix/debug.h>
#include <onix/types.h>

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

extern void console_init();
extern void gdt_init();
extern void interrupt_init();// 中断初始化
extern void clock_init();
extern void hang();
extern void time_init();
extern void rtc_init();
extern void memory_test();
extern void memory_map_init();
extern void mapping_init();

extern void bitmap_test_1();
extern bool interrupt_disable();
extern bool get_interrupt_state();
extern bool set_interrupt_state(bool state);

// 交替开启或关闭中断
void intr_test()
{
    bool intr = interrupt_disable();

    // 设置堆栈
    set_interrupt_state(intr);

}

void kernel_init()
{

    memory_map_init();
    mapping_init();
    interrupt_init();// 中断初始化

    // clock_init();
    // time_init();
    // rtc_init();

    bool intr = interrupt_disable();
    set_interrupt_state(true);
    LOGK("start>>> intr:%d, getintr:%d\n", intr, get_interrupt_state());

    intr = interrupt_disable();
    LOGK("stop>>>> intr:%d, getintr:%d\n", intr, get_interrupt_state());

    set_interrupt_state(true);
    LOGK("start>>> lastintr:%d, getintr:%d\n", intr, get_interrupt_state());

    hang();
    
    // task_init();

    // return;
}