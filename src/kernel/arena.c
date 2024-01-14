#include <onix/arena.h>
#include <onix/memory.h>
#include <onix/string.h>
#include <onix/stdlib.h>
#include <onix/assert.h>
#include <onix/debug.h>

extern u32 free_pages;
static arena_descriptor_t descriptors[DESC_COUNT];

// 主分配区初始化
void arena_init()
{
    u32 block_size = 16;
    // 初始化 16 32 64 128 256 512 1024 的内存块链表
    for (size_t i = 0; i < DESC_COUNT; i++)
    {
        arena_descriptor_t *desc = &descriptors[i];
        desc->block_size = block_size;
        desc->total_block = (PAGE_SIZE - sizeof(arena_t)) / block_size;
        list_init(&desc->free_list);
        block_size <<= 1; // block *= 2
    }
}

// 获取arena 第 idx的内存块指针
static void *get_arena_block(arena_t *arena, u32 idx)
{
    assert(arena->desc->total_block > idx);//当总的内存块idx
    void *addr = (void *)(arena + 1);//拿到主分配区的位置，跳过存储arena结构体的16个字节
    u32 gap = idx * arena->desc->block_size;  
    return addr + gap;// 返回arena的地址
}

// 获取内存块的主分配区的地址
static void *get_block_arena(block_t *block)
{
    return (arena_t *)((u32) block & 0xfffff000);
}

void *kmalloc(size_t size)
{
    arena_descriptor_t *desc = NULL;
    arena_t *arena;
    block_t *block;
    char *addr;

    if (size > 1024)
    {
        u32 asize = size + sizeof(arena_t);
        u32 count = div_round_up(asize, PAGE_SIZE);

        arena = (arena_t *)alloc_kpage(count);
        memset(arena, 0, count * PAGE_SIZE);
        arena->large = true;
        arena->count = count;
        arena->desc = NULL;
        arena->magic = ONIX_MAGIC;

        addr = (char *)((u32) arena + sizeof(arena_t));
        return addr;
    }

    // 找到一个arena 去管理对应大小的内存
    for (size_t i = 0; i < DESC_COUNT; i++)
    {
        desc = &descriptors[i];
        if (desc->block_size >= size)
        {
            break;
        }
    }

    assert(desc != NULL);

    // 如果空闲队列为空，代表没有可分配的内存，这个时候回收内存
    if (list_empty(&desc->free_list))
    {
        arena = (arena_t *)alloc_kpage(1);
        memset(arena, 0, PAGE_SIZE);

        arena->desc = desc;
        arena->large = false;
        arena->count = desc->total_block;
        arena->magic = ONIX_MAGIC;

        for (size_t i = 0; i < desc->total_block; i++)
        {
            block = get_arena_block(arena, i);
            assert(!list_search(&arena->desc->free_list, block));
            list_push(&arena->desc->free_list, block);
            assert(list_search(&arena->desc->free_list, block));
        }
    }

    block = list_pop(&desc->free_list);

    arena = get_block_arena(block);
    assert(arena->magic == ONIX_MAGIC && !arena->large);

    arena->count--;
    return block;        
}


void kfree(void *ptr)
{
    assert(ptr);

    LOGK("kfree %#x\n", ptr);

    block_t *block = (block_t *) ptr;
    arena_t *arena = get_block_arena(block);

    assert(arena->large == true || arena->large == false);
    assert(arena->magic == ONIX_MAGIC);

    // 释放n页物理内存
    if (arena->large)
    {
        free_kpage((u32)arena, arena->count);
        return;
    }

    list_push(&arena->desc->free_list, block);
    arena->count++;

    // 释放对应的内存
    if (arena->count == arena->desc->total_block)
    {
        for (size_t i = 0; i < arena->desc->total_block; i++)
        {
            block = get_arena_block(arena, i);
            assert(list_search(&arena->desc->free_list, block));
            list_remove(block);
            assert(!list_search(&arena->desc->free_list, block));
        }

        free_kpage((u32)arena, 1); //删除
    }  
}