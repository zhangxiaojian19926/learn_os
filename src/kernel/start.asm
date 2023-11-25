[bits 32]

extern kernel_init

global _start
_start:
    call kernel_init
    xchg bx, bx
    ; int 0x80 ; 调用中断函数，系统调用，跳到对应的中断函数进行执行
    mov bx, 0
    div bx
    
    jmp $