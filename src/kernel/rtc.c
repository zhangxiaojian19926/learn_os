
#include <onix/types.h>
#include <onix/debug.h>
#include <onix/interrupt.h>
#include <onix/io.h>
#include <onix/time.h>
#include <onix/assert.h>
#include <onix/stdlib.h>

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

#define CMOS_ADDR 0x70 // CMOS 地址寄存器
#define CMOS_DATA 0x71 // CMOS 数据寄存器

#define CMOS_SECOND 0x01
#define CMOS_MINUTE 0x03
#define CMOS_HOUR 0x05

#define CMOS_A 0x0a
#define CMOS_B 0x0b
#define CMOS_C 0x0c
#define CMOS_D 0x0d
#define CMOS_NMI 0x80

u8 cmos_read(u8 addr)
{
    outb(CMOS_ADDR, CMOS_NMI | addr);
    return inb(CMOS_DATA);
}

// 写cmos寄存器
void cmos_write(u8 addr, u8 value)
{
    outb(CMOS_ADDR, CMOS_NMI | addr);
    outb(CMOS_DATA, value);
}

u32 counter;

void rtc_handler(int vertor)
{
    assert(vertor == 0x28);

    send_eoi(vertor);

    cmos_read(CMOS_C);// 读cmos_c寄存器，允许cmos继续产生中断

    set_alarm(1);// 持续设置为1秒钟触发报警

    LOGK("rtc handler %d......\n", counter++);
}

// 设置 secs 秒后发生实时时钟中断
void set_alarm(u32 secs)
{
    LOGK("beeping after %d seconds\n", secs);

    tm time;
    time_read(&time);// 获取当前时间

    u8 sec = secs % 60;// 剩余的秒
    secs /= 60;// 分钟
    u8 min = secs % 60; // 剩余分钟
    secs /= 60; // 小时
    u32 hour = secs;

    time.tm_sec += sec;
    if (time.tm_sec >= 60)
    {
        time.tm_sec %= 60;
        time.tm_min += 1;
    }

    time.tm_min += min;
    if (time.tm_min >= 60)
    {
        time.tm_min %= 60;
        time.tm_hour += 1;
    }

    time.tm_hour += hour;
    if (time.tm_hour >= 24)
    {
        time.tm_hour %= 24;
    }

    cmos_write(CMOS_HOUR, bin_to_bcd(time.tm_hour));
    cmos_write(CMOS_MINUTE, bin_to_bcd(time.tm_min));
    cmos_write(CMOS_SECOND, bin_to_bcd(time.tm_sec));

    cmos_write(CMOS_B, 0b00100010); // 打开闹钟中断
    cmos_read(CMOS_C);              // 读 C 寄存器，以允许 CMOS 中断
}

// 初始化rtc时钟
void rtc_init()
{
    u8 prev;

    // cmos_write(CMOS_B, 0b01000010); // 打开周期中断
    cmos_write(CMOS_B, 0b00100010); // 打开闹钟中断
    cmos_read(CMOS_C);// 读取寄存器c

    // 设置中断频率
    outb(CMOS_A, (inb(CMOS_A) & 0xf) | 0b1110);

    set_alarm(1);

    // 设置中断函数，打开中断允许
    set_interrupt_handler(IRQ_RTC, rtc_handler);
    set_interrupt_mask(IRQ_RTC, true);
    set_interrupt_mask(IRQ_CASCADE, true);
}