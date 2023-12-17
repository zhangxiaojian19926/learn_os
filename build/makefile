BUILD:=.
SRC:=../src
ROOT:=.

ENTRYPOINT:=0x10000 #内核起始位置

CFLAGS:=-m32 # 32位程序
CFLAGS+=-fno-builtin # 不需要gcc 内置函数，使用自己编译的
CFLAGS+=-nostdinc    # 不能使用c标准的头文件
CFLAGS+=-fno-pic     # 不需要位置无关的代码，position independent code
CFLAGS+=-fno-pie     # 不需要位置无关的可执行程序 position independent execuable
CFLAGS+=-fno-stack-protector  #不需要栈保护
CFLAGS+=$(strip ${CFLAGS})

DEBUG:= -g
INCLUDE:=-I../include/

# 创建boot.bin文件
$(BUILD)/boot/%.bin: $(SRC)/boot/%.asm
	$(shell mkdir -p $(dir $@))
	nasm -f bin  $< -o $@

# 创建kernel文件
$(BUILD)/kernel/%.o: $(SRC)/kernel/%.asm
	$(shell mkdir -p $(dir $@))
	nasm -f elf32  $< $(DEBUG) -o $@

# 编译lib
$(BUILD)/lib/%.o: $(SRC)/../lib/%.c
	$(shell mkdir -p $(dir $@))
	gcc $(CFLAGS) $(DEBUG) $(INCLUDE) -c $< -o $@

# 编译kernel_init
$(BUILD)/kernel/%.o: $(SRC)/kernel/%.c
	$(shell mkdir -p $(dir $@))
	gcc $(CFLAGS) $(DEBUG) $(INCLUDE) -c $< -o $@

# 创建kernel.bin文件
$(BUILD)/kernel/kernel.bin: $(BUILD)/kernel/start.o \
							$(BUILD)/kernel/main.o 	\
							$(BUILD)/kernel/io.o 	\
							$(BUILD)/kernel/console.o\
							$(BUILD)/kernel/printk.o\
							$(BUILD)/kernel/assert.o\
							$(BUILD)/kernel/debug.o\
							$(BUILD)/kernel/global.o\
							$(BUILD)/kernel/task.o \
							$(BUILD)/kernel/schedule.o \
							$(BUILD)/kernel/interrupt.o\
							$(BUILD)/kernel/handler.o \
							$(BUILD)/kernel/clock.o \
							$(BUILD)/kernel/time.o \
							$(BUILD)/kernel/rtc.o \
							$(BUILD)/kernel/memory.o \
							$(BUILD)/lib/string.o	\
							$(BUILD)/lib/bitmap.o	\
							$(BUILD)/lib/vsprintf.o \
							$(BUILD)/lib/stdlib.o	



	$(shell mkdir -p $(dir $@))
	# $^ 表示所有依赖文件 $@  目标文件 静态链接elf_i386的可执行文件
	ld -m elf_i386 -static $^ -o $@ -Ttext $(ENTRYPOINT) 

# 创建system.bin文件
$(BUILD)/kernel/system.bin: $(BUILD)/kernel/kernel.bin
	objcopy -O binary $< $@

# 生成kernel符号  得到符号表，排序之后输入到system.map
$(BUILD)/kernel/system.map: $(BUILD)/kernel/kernel.bin
	nm $< | sort > $@ 

# 创建镜像文件，boot从第1个扇区开始写，loader从第2个扇区开始写，到第4个扇区为止  
$(BUILD)/img/h16M.img: $(BUILD)/boot/boot.bin \
					$(BUILD)/boot/loader.bin  \
					$(BUILD)/kernel/system.bin\
					$(BUILD)/kernel/system.map\

	$(shell mkdir -p $(dir $@))

# 加上&& 才可以进入其他目录执行，创建一个16M的硬盘镜像
	cd $(BUILD)/img && echo "1\nhd\nflat\n512\n16\nh16M.img" | sudo bximage 

# 将boot.bin 写入主引导扇区
	sudo dd if=$(BUILD)/boot/boot.bin of=$@ bs=512 count=1 conv=notrunc

# 将loader.bin 写入硬盘
	sudo dd if=$(BUILD)/boot/loader.bin of=$@ bs=512 count=4 seek=2 conv=notrunc 

# # 测试system.bin 小于100k， 否则需要修改下面的count
# 	test -n "$$(find $(BUILD)/system.bin -size -100k)"

# 将system.bin 写入到硬盘中
	sudo dd if=$(BUILD)/kernel/system.bin of=$@ bs=512 count=200 seek=10 conv=notrunc

test:$(BUILD)/img/h16M.img

.PHONY: clean
clean:
	rm $(BUILD)/boot -rf
	rm $(BUILD)/img -rf
	rm $(BUILD)/kernel -rf


.PHONY:bochs
bochs:$(BUILD)/img/h16M.img
	sudo bochs -f $(BUILD)/bochsrc -unlock

QEMU:=sudo qemu-system-i386 \
			-m 32M \
			-boot c\
			-hda $(BUILD)/img/h16M.img \
			-rtc base=localtime \

.PHONY:qemu
qemu:$(BUILD)/img/h16M.img
	$(QEMU)

.PHONY:qemug
qemug:$(BUILD)/img/h16M.img
	$(QEMU) -s -S

$(BUILD)/img/h16M.vmdk:$(BUILD)/img/h16M.img
	sudo qemu-img convert -O vmdk $< $@

.PHONY:vmdk
vmdk:$(BUILD)/img/h16M.img