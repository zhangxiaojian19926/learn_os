#ifndef ONIX_INTERRUPT_H
#define ONIX_INTERRUPT_H 

#include <onix/types.h>

// 中断描述符表个数
#define IDT_SIZE 256

#define IRQ_CLOCK 0      // 时钟
#define IRQ_KEYBOARD 1   // 键盘
#define IRQ_CASCADE 2    // 8259 从片控制器
#define IRQ_SERIAL_2 3   // 串口 2
#define IRQ_SERIAL_1 4   // 串口 1
#define IRQ_PARALLEL_2 5 // 并口 2
#define IRQ_SB16 5       // SB16 声卡
#define IRQ_FLOPPY 6     // 软盘控制器
#define IRQ_PARALLEL_1 7 // 并口 1
#define IRQ_RTC 8        // 实时时钟
#define IRQ_REDIRECT 9   // 重定向 IRQ2
#define IRQ_NIC 11       // 网卡
#define IRQ_MOUSE 12     // 鼠标
#define IRQ_MATH 13      // 协处理器 x87
#define IRQ_HARDDISK 14  // ATA 硬盘第一通道
#define IRQ_HARDDISK2 15 // ATA 硬盘第二通道

#define IRQ_MASTER_NR 0x20 // 主片起始向量号
#define IRQ_SLAVE_NR 0x28  // 从片起始向量号

typedef void *handler_t; // 中断处理函数


typedef struct gate_t
{
    u16 offset0;    // 段内偏移 0 - 15位
    u16 selector;   // 代码段选择子
    u8  reserved;   // 保留位不用
    u8  type : 4;   // 任务门/中断门/陷阱门
    u8  segment : 1;// segment = 0 表示系统段
    u8  DBL : 2;    // 使用 int 指令访问的最低权限
    u8  present : 1;// 是否有效
    u16 offset1;    // 段内偏移 16 - 31位
} _packed gate_t;

void interrupt_init();

// 通知中断控制器，中断处理结束
void send_eoi(int vector);

// 设置外部中断的中断向量表
void set_interrupt_handler(u32 irq, handler_t handler);

// 设置中断的屏蔽字，动态调整中断屏蔽字，达到动态调整的状态
void set_interrupt_mask(u32 irq, bool enable);

#endif