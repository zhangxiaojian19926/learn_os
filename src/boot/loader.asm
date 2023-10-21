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

    ; 进入保护模式不能调用bios的print
    jmp prepare_protected_mode


prepare_protected_mode:
    xchg bx, bx
    cli;关闭中断

    ; 打开a20线
    in al, 0x92
    or al, 0b10
    out 0x92, al

    lgdt [gdt_ptr];加载gdt，加载全局描述符

    ; 启动保护模式
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ;用跳转刷新缓存，启用保护模式
    jmp dword code_selector:protected_mode

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

error:
    mov si, .msg
    call print
    hlt ;cpu停止
    jmp $
    .msg db "Booting Error!!!", 10, 13, 0

;初始化段寄存器
[bits 32]
protected_mode:
    xchg bx, bx
    mov ax, data_selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax; 

    ; 修改栈顶
    mov esp, 0x10000

jmp $

memory_base equ 0;内存开始位置，基地址位置

;内存界限，能访问的最大内存地址 4G/4k -1
memory_limit equ ((1024 * 1024 * 1024 * 4) / (1024 * 4)) - 1 

code_selector equ (1 << 3)
data_selector equ (2 << 3)

; 描述符指针
gdt_ptr:
    dw (gdt_end - gdt_base) - 1
    dd gdt_base

;全局描述符及地址
gdt_base:
    dd 0, 0

;代码段
gdt_code:
    dw memory_limit & 0xffff;段界限的0-15
    dw memory_base & 0xffff;基地址的0-16
    db (memory_base >> 16) & 0xff;基地址的17-23
    ;存在-dlp 0  代码 - 非依从 - 可读 - 没有被访问 
    db 0b_1_00_1_1_0_1_0
    ; 4k  - 32位 - 不是64位 - 段界限 16 ~19
    db 0b1_1_0_0_0000 | (memory_limit >> 16) & 0xf
    db (memory_base >> 24) & 0xff
gdt_data:
    dw memory_limit & 0xffff;段界限的0-15
    dw memory_base & 0xffff;基地址的0-16
    db (memory_base >> 16) & 0xff;基地址的17-23
    ;存在-dlp 0 -s  数据 - 向上 - 可写 - 没有被访问 
    db 0b_1_00_1_0_0_1_0
    ; 4k  - 32位 - 不是64位 - 段界限 16 ~19
    db 0b1_1_0_0_0000 | (memory_limit >> 16) & 0xf
    db (memory_base >> 24) & 0xff
gdt_end:

;定义结构体数量
ards_count:
    dw 0

;定义结构体
ards_buffer: