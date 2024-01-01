#ifndef ONIX_GLOBAL_H
#define ONIX_GLOBAL_H

#include <onix/types.h>

#define GDT_SIZE 128

#define KERNEL_CODE_IDX 1
#define KERNEL_DATA_IDX 2

#define KERNEL_CODE_SELECTOR (KERNEL_CODE_IDX << 3)
#define KERNEL_DATA_SELECTOR (KERNEL_DATA_IDX << 3)

// 全局描述符
typedef struct descriptor_t /* 共8个字节*/
{
    unsigned short limit_low;     // 段界限 0 -15
    unsigned int base_low : 24;   // 基地址 0 - 23位， 16M
    unsigned char type : 4;       // 段类型
    unsigned char segment : 1;    // 1表示代码段或者数据段，0 表示系统段
    unsigned char DPL :2;         // descroptor privilege level 描述符特权等级，0-3
    unsigned char present : 1;    // 存在位， 1 在内存中， 0 在磁盘上
    unsigned char limit_high : 4; // 段界限 16-19
    unsigned char available : 1;  // 操作系统是否可用标志
    unsigned char long_mode : 1;  // 64位扩展位
    unsigned char big : 1;        // 32位 还是 16位      
    unsigned char granularity:1;  // 粒度 4kb 或 1b
    unsigned char base_high;      // 基地址 24 - 31
} _packed descriptor_t;

// 段选择子
typedef struct selector_t
{
    u8 PRL : 2;
    u8 TI : 1;
    u16 index : 13;
} selector_t;

// 全局描述符 表指针
typedef struct  pointer_t
{
    u16 limit; // pointer_t 大小限制
    u32 base;
}_packed pointer_t;

void gdt_init();

#endif