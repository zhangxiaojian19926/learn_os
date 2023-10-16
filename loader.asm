; 程序加载位置
; 实模式内存布局
[org 0x1000]

dw 0x55aa ;魔数，用于判断错误

mov si, loading
call print
call detect_memory

; 内存检测，知道设备的那块内存能用，哪块内存不能使用
detect_memory:
    ;清空ebx
    xor ebx, ebx

    ;段寄存器清零，es:di结构体缓存位置
    mov ax, 0
    mov es, ax
    mov edi, ards_buffer

    mov edx, 0x534d4150; 固定签名

.next:
    ; 子功能号
    mov eax, 0xe820
    ;ards 结构体大小（字节）
    mov ecx, 20

    ;系统调用0x15
    int 0x15

    ; 如果cf置位，表示出错
    jc error

    ; 将缓存指针指向下一个结构体
    add di, cx

    ;将结构体数量加1
    inc word [ards_count]

    cmp ebx, 0
    jnz .next

    mov si, detecting
    call print

    ; cx表示结构体数量
    mov cx, [ards_count]
    ; 结构体指针
    mov si, 0
; 查看内存地址
; .show:
;     mov eax, [ards_buffer + si]
;     mov ebx, [ards_buffer + si + 8]
;     mov edx, [ards_buffer + si + 16]
;     add si, 20
;     xchg bx, bx
;     loop .show

.done
    ret


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

detecting:
    db "Detecting Memory Success", 10, 13, 0

;定义结构体数量
ards_count:
    dw 0

;定义结构体
ards_buffer:


error:
    mov si, .msg
    call print
    hlt ;cpu停止
    jmp $
    .msg db "Booting Error!!!", 10, 13, 0