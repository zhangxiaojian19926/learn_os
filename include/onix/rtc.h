#ifndef ONIX_RTC_H
#define ONIX_RTC_H

#include <onix/types.h>

u8 cmos_read(u8 addr);
void cmos_write(u8 addr, u8 value);

// 设置 secs 秒后发生实时时钟中断
void set_alarm(u32 secs);

#endif