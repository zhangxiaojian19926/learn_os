#include <onix/debug.h>

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

void kernel_init()
{

    memory_map_init();
    mapping_init();
    interrupt_init();// 中断初始化

    // clock_init();
    // time_init();
    // rtc_init();

    // memory_test();
    bitmap_test_1();

    asm volatile("sti");
    hang();
    
    // task_init();

    // return;
}