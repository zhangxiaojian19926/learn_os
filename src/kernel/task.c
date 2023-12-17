#include <onix/task.h>
#include <onix/printk.h>
#include <onix/debug.h>
#include <onix/memory.h>
#include <onix/assert.h>
#include <onix/interrupt.h>
#include <onix/string.h>
#include <onix/bitmap.h>

#define PAGE_SIZE 0x1000    // 4k的页面
#define NR_TASKS 64         // 最多64个线程
static task_t *task_table[NR_TASKS];

extern bitmap_t kernel_map;
extern void task_switch(task_t *next);

// 从任务列表中，获取可用的任务项
task_t *get_free_task()
{
    for (size_t i = 0; i < NR_TASKS; i++)
    {
        if (NULL == task_table[i])
        {
            task_table[i] = (task_t *)alloc_kpage(1); // 需要free_page(1)
            return task_table[i];// 返回任务的全局地址
        }
    }

    panic("NO more tasks\n");    
}

// 从任务列表中查找指定某种状态的任务
static task_t *task_search(task_state_t state)
{
    assert(!get_interrupt_state());
    task_t *task = NULL;
    task_t *current = running_task();

    for (size_t i = 0; i < NR_TASKS; i++)
    {
        task_t *ptr = task_table[i];
        if (NULL == ptr)
        {
            continue;
        }
        
        if(state != ptr->state)
        {
            continue;
        }

        if (current == ptr)
        {
            continue;
        }

        if (task == NULL || task->ticks < ptr->ticks || ptr->jiffies < task->jiffies)
        {
            task = ptr;
        }  
    }

    return task;
}

task_t *running_task()
{
    asm volatile(
        "movl %esp, %eax\n"
        "andl $0xfffff000, %eax\n"
    );
}

void schedule()
{
    task_t *current = running_task();
    task_t *next = task_search(TASK_READY);

    assert(next != NULL);
    assert(next->magic == ONIX_MAGIC);

    if (TASK_RUNNING == current->state)
    {
        current->state = TASK_READY; // 进程状态需要切换成就绪状态
    }

    next->state = TASK_RUNNING;
    if (next == current) // 当前运行的就是下一个进程则不用切换
    {
        return;
    }

    task_switch(next);
}

static task_t *task_create(target_t target, const char *task_name, u32 priority, u32 uid)
{
    task_t *task = get_free_task();
    memset(task, 0, PAGE_SIZE);

    u32 stack = (u32)task + PAGE_SIZE;

    stack -= sizeof(task_frame_t);
    task_frame_t *frame = (task_frame_t *)stack;
    frame->ebx = 0x11111111;
    frame->esi = 0x22222222;
    frame->edi = 0x33333333;
    frame->ebp = 0x44444444;
    frame->eip = (void *)target;

    strcpy((char *)task->name, task_name);

    task->stack = (u32 *)stack;
    task->priority = priority;
    task->ticks = task->priority;
    task->jiffies = 0;
    task->state = TASK_READY;
    task->uid = uid;
    task->pde = KERNEL_PAGE_DIR;
    task->vmap = &kernel_map;
    task->magic = ONIX_MAGIC;

    return task;
}

static void task_setup()
{
    task_t *task = running_task();
    task->magic = ONIX_MAGIC;
    task->ticks = 1;

    memset(task_table, 0, sizeof(task_table));
}

u32 _ofp thread_a()
{
    set_interrupt_state(true);

    while (true)
    {
        /* code */
        printk("A");
    }  
}

// 省去函数栈帧
u32 _ofp  thread_b()
{
    set_interrupt_state(true);

    while (true)
    {
        /* code */
        printk("B");
    }  
}

// 省去函数栈帧
u32 _ofp  thread_c()
{
    set_interrupt_state(true);

    while (true)
    {
        /* code */
        printk("C");
    }  
}


void task_init()
{
    task_setup();

    task_create(thread_a, "a", 5, KERNEL_USER);
    task_create(thread_b, "b", 5, KERNEL_USER);
    task_create(thread_c, "c", 5, KERNEL_USER);
}