; 程序加载位置
; 实模式内存布局
[org 0x1000]

dw 0x55aa ;魔数，用于判断错误

mov si, loading
call print

; 内存检测，知道设备的那块内存能用，哪块内存不能使用



; 阻塞
jmp $

; print
print:
    mov ah, 0x0e
.next:
    mov al, [si]
    cmp al, 0
    jz .done
    int 0x10
    inc si
    jmp .next
.done:
    ret

; 定义字符串
loading:
    db "Loading Onix...", 10, 13, 0