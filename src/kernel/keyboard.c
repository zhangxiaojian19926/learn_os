#include <onix/interrupt.h>
#include <onix/io.h>
#include <onix/assert.h>
#include <onix/debug.h>

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_CTRL_PORT 0x60

void keyboard_handler(int vector)
{
    assert(vector == 0x21);
    send_eoi(vector);                       // 发送中断处理完成信号
    u16 scancode = inb(KEYBOARD_DATA_PORT); // 读取键盘扫描码
    LOGK("keboard iput %#x\n", scancode);
}

void keyboard_init()
{
    set_interrupt_handler(IRQ_KEYBOARD, keyboard_handler);
    set_interrupt_mask(IRQ_KEYBOARD, true);  
}
