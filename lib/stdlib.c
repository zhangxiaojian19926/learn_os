#include <onix/stdlib.h>

void delay(u32 count)
{
    while (count--)
    {
        /* code */
        ;
    }
    
}

void hang()
{
    while (true)
    {
       ;
    } 
}


// 将bcd码 -> 整数
u8 bcd_to_bin(u8 value)
{
    return (value & 0xf) + (value >> 4) * 10;
}

// 将整数 -> bcd码
u8 bin_to_bcd(u8 value)
{
    return (value / 10) * 0x10 + (value % 10);
}