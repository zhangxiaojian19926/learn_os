#include <onix/memory.h>
#include <onix/types.h>
#include <onix/debug.h>
#include <onix/assert.h>
#include <onix/stdlib.h>
#include <onix/string.h>

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

#define ZONE_VALID 1    // ards 可用内存区域
#define ZONE_RESERVED 2 // ards 不可用区域

#define IDX(addr) ((u32)addr >> 12)  // 获取 addr 的页索引，去掉低12位，得到页索引
#define PAGE(idx) ((u32)idx << 12)   // 获取页索引 idx 对应的页开始的位置
#define ASSERT_PAGE(addr) assert((addr & 0xfff) == 0)

typedef struct ards_t
{
    u64 base;// 内存基地址
    u64 size;// 内存长度
    u32 type;// 内存类型
}_packed ards_t;

static u32 memory_base = 0; // 可用内存基地址，应该等于 1M
static u32 memory_size = 0; // 可用内存大小
static u32 total_pages = 0; // 所有内存页数
static u32 free_pages = 0;  // 空闲内存页数

#define used_pages (total_pages - free_pages) // 已用页数

// 扫描可用内存
void memory_init(u32 magic, u32 addr)
{
    u32 count = 0;
    ards_t *ptr = NULL;

    // 魔数校验
    if (magic == ONIX_MAGIC)
    {
        /* code */
        count = *(u32 *)addr;
        ptr = (ards_t *)(addr + 4);//

        // 找到一个比已有更大的一块内存
        for (size_t i = 0; i < count; i++, ptr++)
        {
            LOGK("Memory base 0x%p size 0x%p type %d\n",
                 (u32)ptr->base, (u32)ptr->size, (u32)ptr->type);

            if (ptr->type == ZONE_VALID && ptr->size > memory_size)
            {
                memory_base = (u32)ptr->base;
                memory_size = (u32)ptr->size;
            }
        }
    }
    else
    {
        panic("memory init magic unknow !!! 0x%p\n", magic);
    }

    LOGK("ARDS count %d\n", count);
    LOGK("Memory base 0x%p\n", (u32)memory_base);
    LOGK("Memory size 0x%p\n", (u32)memory_size);

    assert(memory_base == MEMORY_BASE); // 内存开始的位置为 1M
    assert((memory_size & 0xfff) == 0); // 要求按页对齐，得到的内存没有进行页对齐则会报错

    total_pages = IDX(memory_size) + IDX(MEMORY_BASE);// 加上1M的页数
    free_pages = IDX(memory_size);

    LOGK("Total pages %d\n", total_pages);
    LOGK("Free pages %d\n", free_pages);
}

static u32 start_page = 0;   // 可分配物理内存起始位置
static u8 *memory_map;       // 物理内存数组
static u32 memory_map_pages; // 物理内存数组占用的页数

void memory_map_init()
{
    // 初始化物理内存数组,memory_init得到的物理地址基地址
    memory_map = (u8 *)memory_base;

    // 计算物理内存数组占用的页数
    memory_map_pages = div_round_up(total_pages, PAGE_SIZE);
    LOGK("Memory map page count %d\n", memory_map_pages);

    free_pages -= memory_map_pages;

    // 清空物理内存数组
    memset((void *)memory_map, 0, memory_map_pages * PAGE_SIZE);

    // 前 1M 的内存位置 以及 物理内存数组已占用的页，已被占用，1M以内防止loader.bin 和其他引导相关的东西，
    // 1M之后仅接着用来保存内核
    start_page = IDX(MEMORY_BASE) + memory_map_pages;
    for (size_t i = 0; i < start_page; i++)
    {
        memory_map[i] = 1;//标志已经占用的页的状态，并且不可回收
    }

    LOGK("Total pages %d free pages %d\n", total_pages, free_pages);
}

// 分配一页物理内存
static u32 get_page()
{
    for (size_t i = start_page; i < total_pages; i++)
    {
        // 如果物理内存没有占用
        if (!memory_map[i])
        {
            memory_map[i] = 1;
            assert(free_pages > 0);
            free_pages--;
            u32 page = PAGE(i);
            LOGK("GET page 0x%p\n", page);
            return page;
        }
    }
    panic("Out of Memory!!!");
}

// 释放一页物理内存，提前把物理内存用数组管理起来，之后还就加1，申请之后就减1
static void put_page(u32 addr)
{
    ASSERT_PAGE(addr);

    u32 idx = IDX(addr);

    // idx 大于 1M 并且 小于 总页面数
    assert(idx >= start_page && idx < total_pages);

    // 保证只有一个引用
    assert(memory_map[idx] >= 1);

    // 物理引用减一
    memory_map[idx]--;

    // 若为 0，则空闲页加一
    if (!memory_map[idx])
    {
        free_pages++;
    }

    assert(free_pages > 0 && free_pages < total_pages);
    LOGK("PUT page 0x%p\n", addr);
}

void memory_test()
{
    u32 pages[10];

    for (size_t i = 0; i < 10; i++)
    {
        /* code */
        pages[i] = get_page();
    }

    for (size_t i = 0; i < 10; i++)
    {
        /* code */
        put_page(pages[i]);
    }    
}

// 得到cr3寄存器
u32 get_cr3()
{
    // 直接将 返回值在eax中
    asm volatile("movl %cr3, %eax\n");
}

// 设置cr3 寄存器，参数是页目录的地址
void set_cr3(u32 pde)
{
    ASSERT_PAGE(pde);
    asm volatile("movl %%eax, %%cr3\n" :: "a"(pde));//"a"(pde) 在执行内嵌汇编指令时，将eax的值复制到pde中
}

// 打开分页机制，将cr0的最高位设置成1，开启分页机制
static void enable_page()
{
    // 0b1000_000_000_000_000_0000_000_000
    // 0x80000000
    asm volatile(
        "movl %cr0, %eax\n"
        "orl $0x80000000, %eax\n"
        "movl %eax, %cr0\n"
        );
}

// 初始化页表项
static void entry_init(page_entry_t *entry, u32 index)
{
    *(u32 *)entry = 0;
    entry->present = 1;
    entry->write = 1;
    entry->user = 1;
    entry->index = index;
}

// 内核页目录，4k大小
#define KERNEL_PAGE_DIR 0x200000

// 内核页表, 2M+4k的位置，表示页表所在位置，4k大小
#define KERNEL_PAGE_ENTRY 0x201000

// 初始化内存映射
void mapping_init()
{
    page_entry_t *pde = (page_entry_t *) KERNEL_PAGE_DIR;//页目录地址赋值，首地址
    memset(pde, 0, PAGE_SIZE);

    entry_init(&pde[0], IDX(KERNEL_PAGE_ENTRY));// 页目录的页表型赋值，页表索引赋值

    page_entry_t *pte = (page_entry_t *)KERNEL_PAGE_ENTRY;//页表项初始化
    page_entry_t *entry = NULL;

    // 前1M的内存保存内核，所以也不能被映射出来
    for (size_t tidx = 0; tidx < 1024; tidx++)
    {
        // 对每一个页表项进行初始化
        entry = &pte[tidx];
        entry_init(entry, tidx);
        memory_map[tidx] = 1;
    }

    // 设置cr3寄存器
    set_cr3((u32)pde);

    // 开启分页机制
    enable_page();    
}