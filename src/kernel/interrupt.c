#include <onix/interrupt.h>
#include <onix/global.h>
#include <onix/debug.h>
#include <onix/printk.h>
#include <onix/stdlib.h>
#include <onix/io.h>
#include <onix/assert.h>

#define ENTRY_SIZE 0x30

gate_t idt[IDT_SIZE];
pointer_t idt_ptr;

handler_t handler_table[IDT_SIZE];
extern handler_t handler_entry_table[ENTRY_SIZE];
extern void syscall_handler();

#define PIC_M_CTRL 0x20 // 主片的控制端口
#define PIC_M_DATA 0x21 // 主片的数据端口
#define PIC_S_CTRL 0xa0 // 从片的控制端口
#define PIC_S_DATA 0xa1 // 从片的数据端口
#define PIC_EOI    0x20 // 通知中断控制器中断结束

static char *messages[] = {
    "#DE Divide Error\0",
    "#DB RESERVED\0",
    "--  NMI Interrupt\0",
    "#BP Breakpoint\0",
    "#OF Overflow\0",
    "#BR BOUND Range Exceeded\0",
    "#UD Invalid Opcode (Undefined Opcode)\0",
    "#NM Device Not Available (No Math Coprocessor)\0",
    "#DF Double Fault\0",
    "    Coprocessor Segment Overrun (reserved)\0",
    "#TS Invalid TSS\0",
    "#NP Segment Not Present\0",
    "#SS Stack-Segment Fault\0",
    "#GP General Protection\0",
    "#PF Page Fault\0",
    "--  (Intel reserved. Do not use.)\0",
    "#MF x87 FPU Floating-Point Error (Math Fault)\0",
    "#AC Alignment Check\0",
    "#MC Machine Check\0",
    "#XF SIMD Floating-Point Exception\0",
    "#VE Virtualization Exception\0",
    "#CP Control Protection Exception\0",
};

// 通知中断控制器，中断处理结束
void send_eoi(int vector)
{
    // 通知中断结束
    if(vector >= 0x20 && vector < 0x28)
    {
        outb(PIC_M_CTRL, PIC_EOI);
    }

    if (vector >= 0x28 && vector < 0x30)
    {
        outb(PIC_M_CTRL, PIC_EOI);
        outb(PIC_S_CTRL, PIC_EOI);
    }
}

// 设置外部中断的中断向量表
void set_interrupt_handler(u32 irq, handler_t handler)
{
    assert(irq >= 0 && irq < 16);
    handler_table[IRQ_MASTER_NR + irq] = handler;
}

// 设置中断的屏蔽字，动态调整中断屏蔽字，达到动态调整的状态
void set_interrupt_mask(u32 irq, bool enable)
{
    assert(irq >= 0 && irq < 16);

    u16 port;
    if(irq < 8)
    {
        port = PIC_M_DATA;
    }
    else
    {
        port = PIC_S_DATA;
        irq -= 8;
    }

    if (enable)
    {
        // 先从port读入当前的中断掩码，之后再开启指定的外部中断
        outb(port, inb(port) & ~(1 << irq));
    }
    else
    {
        outb(port, inb(port) | (1 << irq));
    }
}


u32 counter = 0;

void default_handler(int vector)
{
    send_eoi(vector);
    DEBUGK("[%#x] default interrupt called %d...\n", vector, counter++);
}

void exception_handler(
    int vector,
    u32 edi, u32 esi, u32 ebp, u32 esp,
    u32 ebx, u32 edx, u32 ecx, u32 eax,
    u32 gs, u32 fs, u32 es, u32 ds,
    u32 vector0, u32 error, u32 eip, u32 cs, u32 eflags)
{
    char *message = NULL;

    if(vector < 22)
    {
        message = messages[vector];
    }
    else
    {
        message = messages[15];
    }

    // 打印出中断异常时的寄存器信息
    printk("\nEXCEPTION : %s \n", message);
    printk("   VECTOR : 0x%02X\n", vector);
    printk("    ERROR : 0x%08X\n", error);
    printk("   EFLAGS : 0x%08X\n", eflags);
    printk("       CS : 0x%02X\n", cs);
    printk("      EIP : 0x%08X\n", eip);
    printk("      ESP : 0x%08X\n", esp);

    // 阻塞
    hang();
}

// 初始化中断控制器，开启时钟中断 procgrammable Interrupt controller
// 关闭8259的所有中断控制器
void pic_init()
{
    outb(PIC_M_CTRL, 0b00010001); // ICW1: 边沿触发, 级联 8259, 需要ICW4.
    outb(PIC_M_DATA, 0x20);       // ICW2: 起始中断向量号 0x20
    outb(PIC_M_DATA, 0b00000100); // ICW3: IR2接从片.
    outb(PIC_M_DATA, 0b00000001); // ICW4: 8086模式, 正常EOI

    outb(PIC_S_CTRL, 0b00010001); // ICW1: 边沿触发, 级联 8259, 需要ICW4.
    outb(PIC_S_DATA, 0x28);       // ICW2: 起始中断向量号 0x28
    outb(PIC_S_DATA, 2);          // ICW3: 设置从片连接到主片的 IR2 引脚
    outb(PIC_S_DATA, 0b00000001); // ICW4: 8086模式, 正常EOI

    outb(PIC_M_DATA, 0b11111111); // 关闭所有中断
    outb(PIC_S_DATA, 0b11111111); // 关闭所有中断
}

void idt_init()
{
    // 中断描述符，标志中断所在的位置
    for (size_t i = 0; i < ENTRY_SIZE; i++)
    {
        gate_t *gate = &idt[i];
        handler_t handler = handler_entry_table[i];

        gate->offset0 = (u32)handler & 0xffff;
        gate->offset1 = ((u32)handler >> 16) & 0xffff;
        gate->selector = 1 << 3;    // 代码段
        gate->reserved = 0;         // 保留位
        gate->type = 0b1110;        //中断门
        gate->segment = 0;          // 系统段
        gate->DBL = 0;              // 内核态
        gate->present = 1;          // 有效
    }

    // 前32个异常中断处理函数赋值 0x0 - 0x1f
    for (size_t i = 0; i < 0x20; i++)
    {
        handler_table[i] = exception_handler;
    }

    // 外部中断赋值中断处理函数
    for (size_t i = 0x20; i < ENTRY_SIZE; i++)
    {
        handler_table[i] = default_handler;// 外中断默认处理函数
    }

    // 初始化系统调用
    gate_t *gate = &idt[0x80];
    gate->offset0 = (u32)syscall_handler & 0xffff;
    gate->offset1 = ((u32)syscall_handler >> 16) & 0xffff;
    gate->selector = 1 << 3;    // 代码段
    gate->reserved = 0;         // 保留位
    gate->type = 0b1110;        //中断门
    gate->segment = 0;          // 系统段
    gate->DBL = 3;              // 用户态 用户态可以调用
    gate->present = 1;          // 有效
    
    idt_ptr.base = (u32)idt;
    idt_ptr.limit = sizeof(idt) - 1;

    asm volatile(" lidt idt_ptr\n"); //加载 中断描述符表
    
}

// 清除IF标志，返回设置前的值
bool interrupt_disable()
{
    asm volatile(
        "pushfl\n"          // 将当前 eflags 压入栈中
        "cli\n"             // 清除IF位，此时外中断已经屏蔽
        "popl %eax\n"       // 将eflags 弹出到eax
        "shrl $9, %eax\n"   // 将eax 右移 9位，得到IF位
        "andl $1, %eax\n"   // 只需要IF位
    );
}

// 获取IF位，相比interrupt_disable少了cli
bool get_interrupt_state()
{
    asm volatile(
        "pushfl\n"          // 将当前 eflags压入栈中
        "popl %eax\n"       // 将eflags 弹出到eax
        "shrl $9, %eax\n"   // 将eax 右移 9位，得到IF位
        "andl $1, %eax\n"   // 只需要IF位
    );
}

// 设置IF位
void set_interrupt_state(bool state)
{
    if (true == state)
    {
        asm volatile(
            "sti\n" // 打开中断
        );
    }
    else
    {
        asm volatile(
            "cli\n" //关闭中断
        );
    }  
}

// 中断初始化
void interrupt_init()
{
    pic_init();
    idt_init();
}