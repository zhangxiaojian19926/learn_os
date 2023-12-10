#ifndef ONIX_STDLIB_H
#define ONIX_STDLIB_H

#include <onix/types.h>

#define MAX(a, b) ((a) < (b) ? (b) : (a))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

void delay(u32 count);
void hang();


// 将bcd码 -> 整数
u8 bcd_to_bin(u8 value);

// 将整数 -> bcd码
u8 bin_to_bcd(u8 value);

// 除法，向上取整
u32 div_round_up(u32 num, u32 size);

#endif