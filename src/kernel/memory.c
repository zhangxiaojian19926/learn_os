#include <onix/memory.h>
#include <onix/types.h>
#include <onix/debug.h>
#include <onix/assert.h>
#include <onix/stdlib.h>
#include <onix/string.h>
#include <onix/bitmap.h>
#include <onix/multiboot2.h>

#define ZONE_VALID 1    // ards 可用内存区域
#define ZONE_RESERVED 2 // ards 不可用区域

#define IDX(addr) ((u32)addr >> 12)  // 获取 addr 的页索引，去掉低12位，得到页索引
#define PAGE(idx) ((u32)idx << 12)   // 获取页索引 idx 对应的页开始的位置
#define DIDX(addr) (((u32)addr >> 22) & 0x3ff) // 获取 addr 的页目录索引，虚拟地址的前10位是页目录
#define TIDX(addr) (((u32)addr >> 12) & 0x3ff) // 获取 addr 的页表索引，中间10位代表的是页表项
#define ASSERT_PAGE(addr) assert((addr & 0xfff) == 0)

// 内核页bit管理存放位置
#define KERNEL_MAP_BITS 0x4000

// // 内核页表, 2M+4k的位置，表示页表所在位置，4k大小
// #define KERNEL_PAGE_ENTRY 0x201000

#define KERNEL_MEMORY_SIZE (0x100000 * sizeof(KERNEL_PAGE_TABLE))

bitmap_t kernel_map; // 内核map管理

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

    // 魔数校验
    if (magic == ONIX_MAGIC)
    {
        /* code */
        count = *(u32 *)addr;
        ards_t *ptr = (ards_t *)(addr + 4);//

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
    else if (magic == MULTIBOOT2_MAGIC)
    {
        // addr为传进来的物理地址，指定的寄存器传递
        u32 size = *(unsigned int *)addr;
        multi_tag_t *tag = (multi_tag_t *) (addr + 8);// 

        LOGK("Announced mbi size 0x%x \n", size);
        while (tag->type != MULTIBOOT_TAG_TYPE_END)
        {
            if (tag->type == MULTIBOOT_TAG_TYPE_MMAP)
            {
                break;
            }
            // 下一个tag对齐到了8字节
            tag = (multi_tag_t *)((u32)tag + ((tag->size + 7) & ~7));
        }

        // 找到对应能用的物理地址起始位置
        multi_tag_mmap_t *mtag = (multi_tag_mmap_t *)tag;
        multi_mmap_entry_t *entry = mtag->entries;
        while ((u32)entry < (u32)tag + tag->size)
        {
            LOGK("Memory base 0x%p size:0x%p, type:%d\n",
                (u32)entry->addr, (u32)entry->len, (u32)entry->type);
            count++;
            if (entry->type == ZONE_VALID && entry->len > memory_size)
            {
                memory_base = (u32)entry->addr;
                memory_size = (u32)entry->len;
            }
            
            entry = (multi_mmap_entry_t *)((u32)entry + mtag->entry_size);
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

    if (memory_size < KERNEL_MEMORY_SIZE)
    {
        panic("System memory is %dM too small, at least %dM needed\n",
            memory_size / MEMORY_BASE, KERNEL_MEMORY_SIZE / MEMORY_BASE);
    }
    
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
    
    // 初始化内核虚拟内存位图，需要8位对其
    // 8M 内核使用的内存 - 1M 内核 = 剩余页内存
    u32 length = (IDX(KERNEL_MEMORY_SIZE) - IDX(MEMORY_BASE)) / 8;
    bitmap_init(&kernel_map, (u8 *)KERNEL_MAP_BITS, length, IDX(MEMORY_BASE));
    bitmap_scan(&kernel_map, memory_map_pages);
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
static _inline void enable_page()
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

// 初始化内存映射
void mapping_init()
{
    idx_t index = 0;

    page_entry_t *pde = (page_entry_t *) KERNEL_PAGE_DIR;//页目录地址赋值，首地址
    memset(pde, 0, PAGE_SIZE);

    // 对内存映射页进行初始化，内核使用2个页
    for (idx_t didx = 0; didx < (sizeof(KERNEL_PAGE_TABLE) / 4);didx++)
    {
        page_entry_t *pte = (page_entry_t *) KERNEL_PAGE_TABLE[didx];
        memset(pte, 0, PAGE_SIZE);

        page_entry_t *dentry = &pde[didx];
        entry_init(dentry, IDX((u32)pte));//初始化页表项的属性

        // 前1M的内存保存内核，所以也不能被映射出来
        for (size_t tidx = 0; tidx < 1024; tidx++, index++)
        {
            // index 为0的页不映射，为空指针访问报出缺页异常，便于排查
            if (0 == index)
            {
                continue;
            }
            
            // 对内核使用的每一个页表项进行初始化
            page_entry_t *tentry = &pte[tidx];
            entry_init(tentry, index);
            memory_map[index] = 1;
        }
    }

    // 将最后一个页表只想页目录自己，方便修改
    page_entry_t *entry = &pde[1023];
    entry_init(entry, IDX(KERNEL_PAGE_DIR));

    // 设置cr3寄存器，页目录寄存器
    set_cr3((u32)pde);

    // 开启分页机制
    enable_page();    
}

// 获取页目录地址
static page_entry_t *get_pde()
{
    return (page_entry_t *)(0xfffff000);
}

// 获取页表项地址
static page_entry_t *get_pte(u32 vaddr)
{
    return (page_entry_t *)((0xffc00000) | (DIDX(vaddr) << 12));
}

// 刷新指定虚拟地址 vaddr 的快表 TLB
static void flush_tlb(u32 vaddr)
{
    asm volatile("invlpg (%0)" :: "r"(vaddr) 
                 : "memory");
}

// 从位图中扫描 count 个连续的页
static u32 scan_page(bitmap_t *map, u32 count)
{
    assert(count > 0);
    int32 index = bitmap_scan(map, count);

    if (index == EOF)
    {
        panic("Scan page fail!!!");
    }

    u32 addr = PAGE(index);
    LOGK("Scan page 0x%p count %d\n", addr, count);
    return addr;
}

// 与 scan_page 相对，重置相应的页
static void reset_page(bitmap_t *map, u32 addr, u32 count)
{
    ASSERT_PAGE(addr);
    assert(count > 0);
    u32 index = IDX(addr);

    for (size_t i = 0; i < count; i++)
    {
        assert(bitmap_test(map, index + i)); // 确认某一页是否已经分配
        bitmap_set(map, index + i, 0);
    }
}

// 分配 count 个连续的内核页
u32 alloc_kpage(u32 count)
{
    assert(count > 0);
    u32 vaddr = scan_page(&kernel_map, count);
    LOGK("ALLOC kernel pages 0x%p count %d\n", vaddr, count);
    return vaddr;
}

// 释放 count 个连续的内核页
void free_kpage(u32 vaddr, u32 count)
{
    ASSERT_PAGE(vaddr);
    assert(count > 0);
    reset_page(&kernel_map, vaddr, count);
    LOGK("FREE  kernel pages 0x%p count %d\n", vaddr, count);
}

// // 测试memory test 功能
// void memory_test()
// {
//     u32 *pages = (u32 *)(0x200000);
//     u32 count = 0x6fe;
//     for (size_t i = 0; i < count; i++)
//     {
//         /* code */
//         pages[i] = alloc_kpage(1);
//         LOGK("0x%x\n", i);
//     }

//     for (size_t i = 0; i < count; i++)
//     {
//         /* code */
//         free_kpage(pages[i], 1);
//     }
// }

// void memory_test()
// {
//     // 将20M 0x1400000 内存映射到64M 0x4000000 的位置
//     // 页表地址：0x900000

//     BMB;

//     u32 vaddr = 0x4000000; // 虚拟地址可以是任意值
//     u32 paddr = 0x1400000; // 实际的物理地址，物理地址必须存在
//     u32 table = 0x900000;  // 页表也必须是存在的物理地址

//     page_entry_t *pde = get_pde();

//     page_entry_t *dentry = &pde[DIDX(vaddr)];// 页目录中的一项
//     entry_init(dentry, IDX(table));// 获取对应地址的页表目录

//     page_entry_t *pte = get_pte(vaddr);
//     page_entry_t *tentry = &pte[TIDX(vaddr)];

//     entry_init(tentry, IDX(paddr));

//     char *ptr = (char *)(0x4000000);
//     ptr[0] = 'a';

//     entry_init(tentry, IDX(0x1500000));
//     flush_tlb(vaddr);

//     BMB;    
// }
