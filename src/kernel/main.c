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

void kernel_init()
{

    memory_map_init();
    mapping_init();
    interrupt_init();// 中断初始化

    // clock_init();
    // time_init();
    // rtc_init();

    // 未映射的地址，触发缺页错误
    char *ptr = (char *) (0x100000 * 20);// 访问2M的位置
    ptr[0] = 'a';

    asm volatile("sti");
    hang();
    
    // task_init();

    // return;
}