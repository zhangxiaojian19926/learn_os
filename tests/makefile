CURDIR:=.

CFLAGS:= -m32 # 32位程序
CFLAGS+= -Qn
# CFLAGS+=-fno-builtin # 不需要gcc 内置函数，使用自己编译的
# CFLAGS+=-nostdinc    # 不能使用c标准的头文件
CFLAGS+= -fno-pic     # 不需要位置无关的代码，position independent code
# CFLAGS+=-fno-pie     # 不需要位置无关的可执行程序 position independent execuable
# CFLAGS+=-fno-stack-protector  #不需要栈保护
# CFLAGS+=-fomit-frame-pointer    #关闭栈帧
CFLAGS+= -mpreferred-stack-boundary=2#去掉栈对齐
CFLAGS+= -fno-asynchronous-unwind-tables #不需要cfi信息
# CFLAGS+=$(strip ${CFLAGS})

.PHONY: hello.s
hello.s: $(CURDIR)/hello.c
	gcc $(CFLAGS) -S $< -o $@

.PHONY: params.s
params.s: $(CURDIR)/params.c
	gcc $(CFLAGS) -S $< -o $@

.PHONY: clean
clean:
	rm -rf *.o
	rm -rf *.out
