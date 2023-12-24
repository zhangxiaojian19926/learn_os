[bits 32]

extern kernel_init
extern memory_init
extern console_init
extern gdt_init

global _start
_start:

    push ebx; ards_count memory_init传参
    push eax; 魔数

    call console_init ;控制台初始化
    call gdt_init; 初始化全局描述表
    call memory_init; 内存初始化
    call kernel_init;

    jmp $