extern void console_init();
extern void gdt_init();
extern void interrupt_init();// 中断初始化
extern void clock_init();
extern void hang();

void kernel_init()
{
    console_init();

    gdt_init();

    interrupt_init();// 中断初始化

    clock_init();

    asm volatile("sti");
    hang();
    
    // task_init();

    // return;
}