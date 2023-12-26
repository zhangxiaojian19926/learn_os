[bits 32]

section .text

global task_switch
task_switch:
    push ebp
    mov ebp, esp

    push ebx
    push esi
    push edi 

    mov eax, esp
    and eax, 0xfffff000

    mov [eax], esp ;esp保存

    mov eax, [ebp + 8]; next，把下一个任务的栈顶指针取出

    mov esp, [eax] ;取出esp，栈顶指针

    pop edi 
    pop esi
    pop ebx
    pop ebp

    ret