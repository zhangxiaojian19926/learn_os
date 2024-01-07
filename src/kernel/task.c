#include <onix/task.h>
#include <onix/printk.h>
#include <onix/debug.h>
#include <onix/memory.h>
#include <onix/assert.h>
#include <onix/interrupt.h>
#include <onix/string.h>
#include <onix/bitmap.h>
#include <onix/syscall.h>
#include <onix/list.h>
#include <onix/global.h>

#define PAGE_SIZE 0x1000             // 4k的页面
#define NR_TASKS 64                  // 最多64个线程
static task_t *task_table[NR_TASKS]; // 任务表
static list_t block_list;            // 任务默认阻塞表
static list_t sleep_list;            // 任务默认阻塞表
static task_t *idel_task;            // 空闲任务

extern u32 volatile jiffies;// 当前时间片个数
extern u32 jiffy;// 一个时间片的毫秒值
extern tss_t tss;

extern bitmap_t kernel_map;
extern void task_switch(task_t *next);

// 从任务列表中，获取可用的任务项
task_t *get_free_task()
{
    for (size_t i = 0; i < NR_TASKS; i++)
    {
        if (NULL == task_table[i])
        {
            task_table[i] = (task_t *)alloc_kpage(1); // 需要free_kpage(1)
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

    // 空闲任务切换
    if (NULL == task && TASK_READY == state)
    {
        task = idel_task;
    }
    
    return task;
}

void task_yield()
{
    schedule();
}

task_t *running_task()
{
    asm volatile(
        "movl %esp, %eax\n"
        "andl $0xfffff000, %eax\n"
    );
}

// 激活任务
void task_activate(task_t *task)
{
    assert(task->magic == ONIX_MAGIC);

    if (task->uid != KERNEL_USER)
    {
        tss.esp0 = (u32)task + PAGE_SIZE;
    }
}

void schedule()
{
    assert(!get_interrupt_state()); // 不可中断

    task_t *current = running_task();
    task_t *next = task_search(TASK_READY); // 找到一个就绪的任务

    assert(next != NULL);
    assert(next->magic == ONIX_MAGIC);  

    if (TASK_RUNNING == current->state)
    {
        current->state = TASK_READY; // 进程状态需要切换成就绪状态
    }

    // 时钟周期变回初值，即优先级的概念
    if(0 == current->ticks)
    {
        current->ticks = current->priority;
    }

    next->state = TASK_RUNNING;
    if (next == current) // 当前运行的就是下一个进程则不用切换
    {
        return;
    }

    task_activate(next);
    task_switch(next);
}

static task_t *task_create(target_t target, const char *task_name, u32 priority, u32 uid)
{
    task_t *task = get_free_task();
    memset(task, 0, PAGE_SIZE);

    u32 stack = (u32)task + PAGE_SIZE;

    stack -= sizeof(task_frame_t); // 将地址往前移20个字节
    task_frame_t *frame = (task_frame_t *)stack;
    frame->ebx = 0x11111111;
    frame->esi = 0x22222222;
    frame->edi = 0x33333333;
    frame->ebp = 0x44444444;
    frame->eip = (void *)target; // 函数地址

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

// 调用该函数的地方不能有任何局部变量
// 调用前栈顶需要准备足够的空间
void task_to_user_mode(target_t target)
{
    task_t *task = running_task();

    u32 addr = (u32)task + PAGE_SIZE;

    addr -= sizeof(intr_frame_t);
    intr_frame_t *iframe = (intr_frame_t *)(addr);

    iframe->vector = 0x20;
    iframe->edi = 1;
    iframe->esi = 2;
    iframe->ebp = 3;
    iframe->esp_dummy = 4;
    iframe->ebx = 5;
    iframe->edx = 6;
    iframe->ecx = 7;
    iframe->eax = 8;

    iframe->gs = 0;
    iframe->ds = USER_DATA_SELECTOR;
    iframe->es = USER_DATA_SELECTOR;
    iframe->fs = USER_DATA_SELECTOR;
    iframe->ss = USER_DATA_SELECTOR;
    iframe->cs = USER_CODE_SELECTOR;

    iframe->error = ONIX_MAGIC;

    u32 stack3 = alloc_kpage(1); // 申请用户栈的内存

    iframe->eip = (u32)target;
    iframe->eflags = (0 << 12 | 0b10 | 1 << 9);
    iframe->esp = stack3 + PAGE_SIZE;

    // ROP 技术，直接从中断返回
    // 通过 eip 跳转到 entry 执行
    asm volatile(
        "movl %0, %%esp\n"
        "jmp interrupt_exit\n" ::"m"(iframe));
}

// 任务枷锁与解锁过程
void task_block(task_t *task, list_t *blist, task_state_t state)
{
    assert(!get_interrupt_state());
    assert(task->node.next == NULL);
    assert(task->node.prev == NULL);

    if (NULL == blist)
    {
        blist = &block_list;
    }

    list_push(blist, &task->node);
    assert(state != TASK_READY && state != TASK_RUNNING); // 需要阻塞的任务不能是正在准备或运行的任务

    task->state = state;

    task_t *current = running_task();
    if(current == task)
    {
        schedule();
    } 
}

void task_unblock(task_t *task)
{
    assert(!get_interrupt_state());

    list_remove(&task->node);

    assert(task->node.next == NULL);
    assert(task->node.prev == NULL);

    task->state = TASK_READY;
}

static void task_setup()
{
    task_t *task = running_task();
    task->magic = ONIX_MAGIC;
    task->ticks = 1;

    memset(task_table, 0, sizeof(task_table));
}

void task_sleep(u32 ms)
{
    assert(!get_interrupt_state()); // 不可中断

    u32 ticks = ms / jiffy; //需要多少个时间片
    ticks = ticks > 0 ? ticks : 1; //至少一个时间片的休眠

    // 记录目标全局时间片，在指定时刻唤醒线程
    task_t *current = running_task();
    current->ticks = jiffies + ticks;// 当前线程应该到哪个时间开始被唤醒

    // 从睡眠链表中找到第一个比当前任务唤醒时间点更晚的任务，进行插入排序
    list_t *list = &sleep_list;
    list_node_t *anchor = &list->tail;

    for (list_node_t *ptr = list->head.next; ptr != &list->tail; ptr = ptr->next)
    {
        task_t *task = element_entry(task_t, node, ptr);
        
        // 找到一个剩余的剩余时间片数更大的一个任务，
        // 并把当前的任务插在这个任务前面
        if (task->ticks > current->ticks)
        {
            anchor = ptr;
            break;
        }
    }

    assert(current->node.next == NULL);
    assert(current->node.prev == NULL);

    // 插入链表
    // 插到当前列表的前面
    list_insert_before(anchor, &current->node);

    // 阻塞状态时睡眠
    // 插入之后，当前的任务设置为睡眠状态
    current->state = TASK_SLEEPING;

    schedule(); 
}

void task_wakeup()
{
    assert(!get_interrupt_state());

    //jiffies指的时当前已经过去的时间片数
    // 从睡眠队列中找到ticks小于或等于jiffies的任务，恢复执行
    list_t *list = &sleep_list;
    for(list_node_t *ptr = list->head.next; ptr != &list->tail; )
    {
        task_t *task = element_entry(task_t, node, ptr);

        // 小于或等于jiffies的任务可以被唤醒
        // 所有的剩余时间片数大于当前已经运行的时间片总数的任务都不能被唤醒
        if (task->ticks > jiffies)
        {
            break;
        }

        // unblock 会将指针清空，唤醒小于jiffies的线程，即唤醒所有应该唤醒的新城
        ptr = ptr->next;

        task->ticks = 0;
        task_unblock(task); 
    }
}

extern void idle_thread();
extern void init_thread();
extern void test_thread();

void task_init()
{
    list_init(&block_list);
    list_init(&sleep_list);// 初始化睡眠列表
    task_setup();

    idel_task = task_create(idle_thread, "idle", 1, KERNEL_USER);
    task_create(init_thread, "init", 5, NORMAL_USER);// 用户层线程
    task_create(test_thread, "test", 5, NORMAL_USER);// 用户层线程
}