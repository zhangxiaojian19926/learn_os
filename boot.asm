[org 0x7c00]

;设置文本模式为文本模式，清除屏幕
mov ax, 3
int 0x10

; 初始化段寄存器
mov ax, 0
mov ds, ax
mov es, ax
mov ss, ax
mov sp, 0x7c00


; 0xb800 文本显示区域
mov si, booting
call print

mov edi, 0x1000;读的目标内存
mov ecx, 0;起始扇区
mov bl, 1;扇区数量
call read_disk

mov edi, 0x1000;写的目标内存
mov ecx, 2;起始扇区
mov bl, 1;扇区数量
call write_disk

; bochs 魔数断点
xchg bx, bx

;阻塞
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

; 读硬盘数据
read_disk:
    ;设置读写扇区的数量
    mov dx, 0x1f2
    mov al, bl
    out dx, al

    inc dx;0x1f3
    mov al, cl;起始扇区的前八位
    out dx, al

    inc dx;0x1f4
    shr ecx, 8
    mov al, cl;起始扇区的中八位
    out dx, al

    inc dx;0x1f5
    shr ecx, 8
    mov al, cl;起始扇区的高八位
    out dx, al

    inc dx;0x1f6
    shr ecx, 8
    and cl, 0b1111;将高4位置成0

    mov al, 0b1110_0000;
    or al, cl
    out dx, al;主盘，LBA模式

    inc dx;0x1f7
    mov al, 0x20;读硬盘
    out dx, al

    xor ecx, ecx;清空ecx
    mov cl, bl;得到读写扇区的数量

    .read:
        push cx; 保存cx
        call .waits;等待数据准备完毕
        call .reads;读取一个扇区
        pop cx
        loop .read;循环

    ret

;等待数据准备完毕
.waits:
    mov dx, 0x1f7
    .check:
        in al, dx
        jmp $+2;nop 直接跳到下一行
        jmp $+2;延迟
        jmp $+2 
        and al, 0b1000_1000
        cmp al, 0b0000_1000
        jnz .check
        ret

.reads:
    mov dx, 0x1f0
    mov cx, 256;一个扇区 256个字节
    .readw:
        in ax, dx
        jmp $+2 
        jmp $+2 
        jmp $+2 
        mov [edi], ax
        add edi, 2
        loop .readw
    ret


; 将读出来的数据写入到第二个扇区
write_disk:
    ;设置读写扇区的数量
    mov dx, 0x1f2
    mov al, bl
    out dx, al

    inc dx;0x1f3
    mov al, cl;起始扇区的前八位
    out dx, al

    inc dx;0x1f4
    shr ecx, 8
    mov al, cl;起始扇区的中八位
    out dx, al

    inc dx;0x1f5
    shr ecx, 8
    mov al, cl;起始扇区的高八位
    out dx, al

    inc dx;0x1f6
    shr ecx, 8
    and cl, 0b1111;将高4位置成0

    mov al, 0b1110_0000;
    or al, cl
    out dx, al;主盘，LBA模式

    inc dx;0x1f7
    mov al, 0x30;写硬盘
    out dx, al

    xor ecx, ecx;清空ecx
    mov cl, bl;得到读写扇区的数量

    .write:
        push cx; 保存cx
        call .writes;读取一个扇区
        call .waits;等待硬盘繁忙结束
        pop cx
        loop .write;循环

    ret

;等待数据准备完毕
.waits:
    mov dx, 0x1f7
    .check:
        in al, dx
        jmp $+2;nop 直接跳到下一行
        jmp $+2;延迟
        jmp $+2 
        and al, 0b1000_0000
        cmp al, 0b0000_0000
        jnz .check
        ret

.writes:
    mov dx, 0x1f0
    mov cx, 256;一个扇区 256个字节
    .writew:
        mov ax, [edi]  
        out dx, ax
        jmp $+2 
        jmp $+2 
        jmp $+2 
        add edi, 2
        loop .writew
    ret

; 10->\n, 13->\r 
booting:
    db "zhang Booting Onix...", 10, 13, 0

; 填充0
times 510 - ($ - $$) db 0

; bios 要求，主引导扇区的最后两个字节必须是0x55 0xaa
db 0x55, 0xaa

