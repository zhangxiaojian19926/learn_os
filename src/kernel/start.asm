[bits 32]

extern kernel_init
extern memory_init
extern console_init

global _start
_start:
    ; call kernel_init

    push ebx; ards_count
    push eax; 魔数
    call console_init ;控制台初始化
    call memory_init; 内存初始化

    jmp $