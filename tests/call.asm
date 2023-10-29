; [bit 32]

extern exit

; 把当前行压入堆栈
test: 
    push $ 
    ret

global main
main:
    push 5
    push eax

    pop ebx
    pop ecx

    pusha
    popa

    call test

    push 0
    call exit