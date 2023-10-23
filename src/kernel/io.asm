[bits 32]

; 代码段
section .text 

; 将inb导出
global inb 

inb:
    push ebp
    mov ebp, esp 
    xor eax, eax ; 清空eax
    mov edx, [ebp + 8] ; 调用函数将port压入栈中
    in al, dx ; 将dx的8bit输入到al中
    jmp $+2  ;一点点延迟
    jmp $+2  ;一点点延迟
    jmp $+2  ;一点点延迟

    leave ; 恢复栈帧
    ret

global outb 
outb:
    push ebp    
	mov ebp, esp; 保存栈帧

    mov edx, [ebp + 8]; port
    mov eax, [ebp + 12];value

    out dx, al; 将al中的8bit 输入到dx

    jmp $+2  ;一点点延迟
    jmp $+2  ;一点点延迟
    jmp $+2  ;一点点延迟

    leave; 恢复栈帧
    ret

global inw 
inw:
    push ebp    
	mov ebp, esp; 保存栈帧

    mov edx, [ebp + 8] ; 调用函数将port压入栈中
    in ax, dx ; 将dx的16bit输入到ax中

    jmp $+2  ;一点点延迟
    jmp $+2  ;一点点延迟
    jmp $+2  ;一点点延迟

    leave; 恢复栈帧
    ret


global outw 
outw:
    push ebp    
	mov ebp, esp; 保存栈帧

    mov edx, [ebp + 8]; port
    mov eax, [ebp + 12];value

    out dx, ax; 将ax中的8bit 输入到dx

    leave; 恢复栈帧
    ret