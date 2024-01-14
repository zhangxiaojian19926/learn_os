#ifndef ONIX_ARENA_H
#define ONIX_ARENA_H

#include <onix/types.h>
#include <onix/list.h>

#define DESC_COUNT 7

typedef list_node_t block_t;// 内存块

// 内存块描述符
typedef struct arena_descriptor_t
{
    u32 total_block;    // 一页内存分成多少块
    u32 block_size;     // 块大小 
    list_t free_list;   // 空闲列表
}arena_descriptor_t;

// 一页或者多页内存， arena 舞台，划分不同的块
typedef struct arena_t
{
    arena_descriptor_t *desc;   // 该arena的描述符
    u32 count;                  // 当前剩余多少块
    u32 large;                  // 表示是不是超过了大小
    u32 magic;                  // 魔数
}arena_t;

void *kmalloc(size_t size);
void kfree(void *ptr);

#endif