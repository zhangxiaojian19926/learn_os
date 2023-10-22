# gcc 汇编分析

## CFI

call frame information / 调用栈帧信息

## PIC 

position independent code / 位置无关的代码

    call	__x86.get_pc_thunk.ax


## 代码分析

```asm
	.file	"hello.c"  # 文件名
.text    # 代码段
	.globl	message
.data #数据段
	.align 4
	.type	message, @object
	.size	message, 16 #message的尺寸
message: # message的定义
	.string	"hello world!!!\n"
	.comm	buf,1024,32
.text # 代码段
	.globl	main
	.type	main, @function
main:
	endbr32
	pushl	$message # 将message压入到栈中
	call	printf
	addl	$4, %esp # 恢复栈
	movl	$0, %eax # main函数返回值，存储在eax中
	ret #函数调用返回
	.size	main, .-main # main函数尺寸
	.section	.note.GNU-stack,"",@progbits #标记栈不可运行
	.section	.note.gnu.property,"a"
	.align 4
	.long	 1f - 0f
	.long	 4f - 1f
	.long	 5
0:
	.string	 "GNU"
1:
	.align 4
	.long	 0xc0000002
	.long	 3f - 2f
2:
	.long	 0x3
3:
	.align 4
4:
```