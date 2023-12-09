#include <onix/memory.h>
#include <onix/types.h>
#include <onix/debug.h>
#include <onix/assert.h>

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

#define ZONE_VALID 1    // ards 可用内存区域
#define ZONE_RESERVED 2 // ards 不可用区域

#define IDX(addr) ((u32)addr >> 12)  // 获取 addr 的页索引，去掉低12位，得到页索引

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

    total_pages = IDX(memory_size) + IDX(MEMORY_BASE);// 通过索引获取总的页数
    free_pages = IDX(memory_size);

    LOGK("Total pages %d\n", total_pages);
    LOGK("Free pages %d\n", free_pages);
}
