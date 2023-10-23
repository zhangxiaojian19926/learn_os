#ifndef ONIX_IO_H
#define ONIX_IO_H

#include <onix/types.h>

extern u8 inb(u16 port); //输入一个字符
extern u16 inw(); //输入一个子

extern void outb(u16 port, u8 value); //输出一个字节
extern void outw(u16 prot, u16 value); // 输出一个字

#endif