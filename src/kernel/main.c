#include <onix/debug.h>
#include <onix/types.h>

#include <onix/task.h>

extern void console_init();
extern void gdt_init();
extern void interrupt_init();// 中断初始化
extern void clock_init();
extern void task_init();
extern void hang();
extern void time_init();
extern void rtc_init();
extern void memory_test();
extern void memory_map_init();
extern void mapping_init();
extern void syscall_init();
extern void keyboard_init();

extern void bitmap_test_1();
extern bool interrupt_disable();
extern bool get_interrupt_state();
extern bool set_interrupt_state(bool state);

void kernel_init()
{
    memory_map_init();
    mapping_init();
    interrupt_init();// 中断初始化

    clock_init();
    keyboard_init();//初始化键盘

    task_init();
    syscall_init();

    set_interrupt_state(true);

    // time_init();
    // rtc_init();

    // hang()

    // return;
}