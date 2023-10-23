#include <onix/onix.h>
#include <onix/types.h>
#include <onix/io.h>

// CRT 地址寄存器 0x3D4
// CRT 数据寄存器 0x3D5
// CRT 光标位置 - 高位 0xE
// CRT 光标位置 - 低位 0xF

#define CRT_ADDR_REG 0x3D4
#define CRT_DATA_REG 0x3D5

#define CRT_CURSOR_REG_H 0xE
#define CRT_CURSOR_REG_L 0xF

void kernel_init()
{
    outb(CRT_ADDR_REG, CRT_CURSOR_REG_H);
    u16 pos = inb(CRT_DATA_REG) << 8;
    outb(CRT_ADDR_REG, CRT_CURSOR_REG_L);
    pos |= inb(CRT_DATA_REG);//低高8位组成地址，屏幕每一行都是80个字节

    outb(CRT_ADDR_REG, CRT_CURSOR_REG_H);
    outb(CRT_DATA_REG, 0);// 改变光标高位置
    outb(CRT_ADDR_REG, CRT_CURSOR_REG_L);
    outb(CRT_DATA_REG, 0);// 改变光标低位置，写0xf

    u8 data = inb(CRT_ADDR_REG);
}