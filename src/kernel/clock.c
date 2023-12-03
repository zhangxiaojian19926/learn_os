#include <onix/io.h>
#include <onix/interrupt.h>
#include <onix/assert.h>
#include <onix/debug.h>
#include <onix/rtc.h>

#define PIT_CHAN0_REG 0x40
#define PIT_CHAN2_REG 0x42 //用于输出不同音调的声音
#define PIT_CTRL_REG  0x43

#define HZ 100
#define OSCILLATOR 1193182
#define CLOCK_COUNTER (OSCILLATOR / HZ)
#define JIFFY (1000 / HZ)

#define SPEAKER_REG 0x61 
#define BEEP_HZ 440
#define BEEP_COUNTER (OSCILLATOR / BEEP_HZ)

//时间片计算
u32 volatile jiffies = 0;
u32 jiffy = JIFFY;

u32 volatile beeping = 0;

void start_beep()
{
    if (!beeping)
    {
        outb(SPEAKER_REG, inb(SPEAKER_REG) | 0b11);
    }

    beeping = jiffies + 5;    
}

void stop_beep()
{
    if (beeping && jiffies > beeping)
    {
        outb(SPEAKER_REG, inb(SPEAKER_REG) & 0xfc);
        beeping = 0;
    }
}

void clock_handler(int vertor)
{
    assert(vertor == 0x20);
    send_eoi(vertor);

    if (jiffies % 200 == 0)
    {
        DEBUGK("clock jiffies %d\n", jiffies);
        start_beep();
    }

    jiffies++;
    // DEBUGK("clock jiffies %d\n", jiffies);

    stop_beep();
}

// 初始化时钟计数器,从CLOCK_COUNTER减到零，之后又从CLOCK_COUNTER开始见到零，周而复始
void pit_init()
{
    // 配置计数器0 时钟
    outb(PIT_CTRL_REG, 0b00110100);
    outb(PIT_CHAN0_REG, CLOCK_COUNTER & 0xff);
    outb(PIT_CHAN0_REG, (CLOCK_COUNTER >> 8) & 0xff);

    // 配置计数器2 蜂鸣器
    outb(PIT_CTRL_REG, 0b10110110);
    outb(PIT_CHAN2_REG, BEEP_COUNTER & 0xff);
    outb(PIT_CHAN2_REG, (BEEP_COUNTER >> 8) & 0xff);
}

// 时钟中断处理注册到对应的中断向量表中
void clock_init()
{
    pit_init();
    set_interrupt_handler(IRQ_CLOCK, clock_handler);
    set_interrupt_mask(IRQ_CLOCK, true);    
}


