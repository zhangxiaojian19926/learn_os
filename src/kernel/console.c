#include <onix/console.h>
#include <onix/io.h>
#include <onix/string.h>
#include <onix/interrupt.h>

#define CRT_ADDR_REG 0x3D4 // CRT(6845)索引寄存器
#define CRT_DATA_REG 0x3D5 // CRT(6845)数据寄存器

#define CRT_START_ADDR_H 0xC // 显示内存起始位置 - 高位
#define CRT_START_ADDR_L 0xD // 显示内存起始位置 - 低位
#define CRT_CURSOR_H 0xE     // 光标位置 - 高位
#define CRT_CURSOR_L 0xF     // 光标位置 - 低位

#define MEM_BASE 0xB8000              // 显卡内存起始位置
#define MEM_SIZE 0x4000               // 显卡内存大小
#define MEM_END (MEM_BASE + MEM_SIZE) // 显卡内存结束位置
#define WIDTH  80                     // 屏幕文本列数
#define HEIGHT 25                     // 屏幕文本行数
#define ROW_SIZE (WIDTH * 2)          // 每行字节数，第一个字节是acsii，第二个是样式字节
#define SCR_SIZE (ROW_SIZE * HEIGHT)  // 屏幕字节数

#define NUL 0x00
#define ENQ 0x05
#define ESC 0x1B // ESC
#define BEL 0x07 // \a
#define BS  0x08  // \b
#define HT  0x09  // \t
#define LF  0x0A  // \n
#define VT  0x0B  // \v
#define FF  0x0C  // \f
#define CR  0x0D  // \r
#define DEL 0x7F

static u32 screen; // 记录屏幕字节写的位置
static u32 pos;    //记录当前光标位置
static u32 x, y;       // 当前光标位置
static u8 attr = 7; // 字符样式
static u16 erase = 0x0720;//空格，删除是填写

// 获取屏幕开始当前位置
static void get_screen()
{
    outb(CRT_ADDR_REG, CRT_START_ADDR_H);
    screen = inb(CRT_DATA_REG) << 8;
    outb(CRT_ADDR_REG, CRT_START_ADDR_L);
    screen |= inb(CRT_DATA_REG);  

    screen <<= 1; // screen ** 2
    screen += MEM_BASE;
}

// 设置屏幕开始位置到screen
static void set_screen()
{
    outb(CRT_ADDR_REG, CRT_START_ADDR_H);
    outb(CRT_DATA_REG, ((screen - MEM_BASE) >> 9) & 0xff);
    outb(CRT_ADDR_REG, CRT_START_ADDR_L);
    outb(CRT_DATA_REG, ((screen - MEM_BASE) >> 1) & 0xff);
}

// 获取屏幕当前光标位置
static void get_cursor()
{
    outb(CRT_ADDR_REG, CRT_CURSOR_H);
    pos = inb(CRT_DATA_REG) << 8;
    outb(CRT_ADDR_REG, CRT_CURSOR_L);
    pos |= inb(CRT_DATA_REG); 

    get_screen();

    pos <<= 1;
    pos += MEM_BASE;

    u32 delta = (pos - screen) >> 1;//获取x, y
    x = delta % WIDTH;
    y = delta / WIDTH; 
}

// 设置屏幕当前光标位置
static void set_cursor()
{
    outb(CRT_ADDR_REG, CRT_CURSOR_H);
    outb(CRT_DATA_REG, ((pos - MEM_BASE) >> 9) & 0xff); // 高8位
    outb(CRT_ADDR_REG, CRT_CURSOR_L);
    outb(CRT_DATA_REG, ((pos - MEM_BASE) >> 1) & 0xff);
}

// 向上滚动一行
static void scroll_up()
{
    if(screen + SCR_SIZE + ROW_SIZE >= MEM_END)
    {
        memcpy((void*)MEM_BASE, (void *)screen, SCR_SIZE);
        pos -= (screen - MEM_BASE);
        screen = MEM_BASE;
    }

    u32 *ptr = (u32 *) (screen + SCR_SIZE);
    for(size_t i = 0; i < WIDTH; i++)
    {
        *ptr++ = erase;
    }
    screen += ROW_SIZE;
    pos    += ROW_SIZE;

    set_screen();//设置屏幕开始的位置
}

// 换行
static void command_lf()
{
    if (y + 1 < HEIGHT)
    {
        y++;
        pos += ROW_SIZE;
        return;
    }

    scroll_up();
}

//回到当前字符串的开头
static void command_cr()
{
    pos -= (x << 1);//当前回到最开始的位置
    x = 0;
}

// 删除前一个字符
static void command_bs()
{
    if(x)
    {
        x--;
        pos -= 2;
        *(u16 *)pos = erase;//清除前一个字符
    }
}

//删除当前字符
static void command_del()
{
    *(u16 *) pos = erase;
}

extern void start_beep();

// 写控制台，将字符写入
void console_write(char * buf, u32 count)
{
    bool intr = interrupt_disable();
    char ch;

    while (count--)
    {
        ch = *buf++;

        switch (ch)
        {
            case NUL:
                /* code */
                break;
            case ENQ: 
                
                break;
            case ESC:  // ESC
                
                break;
            case BEL:  // \a
                start_beep();
                //to do
                break;
            case BS :  // \b，退格键
                command_bs();
                break;
            case HT :  // \t
                //to do 
                break;
            case LF :  // \n，换行
                command_lf();
                command_cr();//回到当前开头
                break;
            case VT :  // \v
                // to do
                break;
            case FF :  // \f
                command_lf();
                break;
            case CR :  // \r
                command_cr();
                break;
            case DEL:  // 删除
                command_del();
                break;
            
            default:
                if(x >= WIDTH)
                {
                    x -= WIDTH;
                    pos -= ROW_SIZE;
                    command_lf();
                }
                
                *((char *) pos) = ch;
                pos++;
                *((char *) pos) = attr;
                pos++;

                x++;
                break;
        }
    }

    set_cursor();// 根据pos设置光标位置，与屏幕内存位置无光

    set_interrupt_state(intr); // 产生原子操作，保护临界区操作
}

void console_clear()
{
    screen = MEM_BASE;
    pos = MEM_BASE;
    x = y = 0;

    // 从屏幕最开始位置一直往下清除
    set_cursor();
    set_screen();

    u16 * ptr = (u16 * )MEM_BASE;

    while (ptr < (u16 * )MEM_END)
    {
        *ptr++ = erase;
    }
    
}

void console_init()
{
    console_clear();
    // get_screen();
    // pos = 80 * 2 * 2 + MEM_BASE;

    // get_cursor();
    // set_cursor();
}