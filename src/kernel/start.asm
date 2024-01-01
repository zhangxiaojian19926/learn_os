[bits 32]

magic  equ 0xe85250d6
i386   equ 0
length equ header_end - header_start

section .multiboot2
header_start:
    dd magic    ;魔数
    dd i386     ;32位保护模式
    dd length   ;头部长度
    dd -(magic + i386 + length); 校验和

    ; 结束标记
    dw 0    ; type
    dw 0    ; flags
    dw 8    ; size
header_end

extern kernel_init
extern memory_init
extern console_init
extern gdt_init
extern gdt_ptr

code_selector equ (1 << 3)
data_selector equ (2 << 3)

section .text
global _start
_start:

    push ebx; ards_count memory_init传参
    push eax; 魔数

    call console_init ;控制台初始化
    call gdt_init; 初始化全局描述表
    lgdt [gdt_ptr];加载全局描述符
    jmp dword code_selector:_next;跳转到指定的代码段选择子，并开始从那里执行代码。这是加载和开始执行内核代码的标志

_next:
    mov ax, data_selector
    ;为了确保系统使用正确的内存段来读取和写入数据
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov gs, ax
    mov ss, ax; 初始化段寄存器

    call memory_init; 内存初始化
    mov esp, 0x10000; 修改栈顶, 这是为了确保栈有一个合适的起始位置
    call kernel_init;这通常是内核的最后初始化步骤，包括启动任务、多线程等

    jmp $