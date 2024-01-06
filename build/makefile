BUILD:=.
SRC:=../src
ROOT:=.

MULTIOOT2:=0x10000 #multiboot2 开始位置
ENTRYPOINT:=$(shell python3 -c "print(f'0x{$(MULTIOOT2) + 64:x}')") # 根据multiboot2的所占用的空间计算当前内核的内存位置

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

LDFLAGS:= -m elf_i386 \
		  -static \
		  -Ttext $(ENTRYPOINT) \
		  --section-start=.multiboot2=$(MULTIOOT2)

LDFLAGS:=$(strip ${LDFLAGS}) # 让LD参数好看

# 创建kernel.bin文件
$(BUILD)/kernel/kernel.bin: $(BUILD)/kernel/start.o \
							$(BUILD)/kernel/main.o 	\
							$(BUILD)/kernel/io.o 	\
							$(BUILD)/kernel/console.o\
							$(BUILD)/kernel/printk.o\
							$(BUILD)/kernel/assert.o\
							$(BUILD)/kernel/debug.o\
							$(BUILD)/kernel/thread.o\
							$(BUILD)/kernel/global.o\
							$(BUILD)/kernel/task.o \
							$(BUILD)/kernel/gate.o \
							$(BUILD)/kernel/mutex.o \
							$(BUILD)/kernel/schedule.o \
							$(BUILD)/kernel/interrupt.o\
							$(BUILD)/kernel/handler.o \
							$(BUILD)/kernel/clock.o \
							$(BUILD)/kernel/time.o \
							$(BUILD)/kernel/rtc.o \
							$(BUILD)/kernel/memory.o \
							$(BUILD)/kernel/keyboard.o \
							$(BUILD)/lib/string.o	\
							$(BUILD)/lib/bitmap.o	\
							$(BUILD)/lib/vsprintf.o \
							$(BUILD)/lib/stdlib.o	\
							$(BUILD)/lib/syscall.o	\
							$(BUILD)/lib/list.o	

	$(shell mkdir -p $(dir $@))
	# $^ 表示所有依赖文件 $@  目标文件 静态链接elf_i386的可执行文件
	ld ${LDFLAGS} $^ -o $@  

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

$(BUILD)/img/kernel.iso: $(BUILD)/kernel/kernel.bin $(SRC)/utils/grub.cfg

# 新建img目录
	$(shell mkdir -p $(dir $@))
# 检查内存是否合法，检查kernel.bin是否是x86的文件
	grub-file --is-x86-multiboot2 $<
#创建 iso 目录
	mkdir -p $(BUILD)/iso/boot/grub
#拷贝内核文件
	cp $< $(BUILD)/iso/boot
#拷贝grub的配置文件
	cp $(SRC)/utils/grub.cfg $(BUILD)/iso/boot/grub
#生成 iso 文件
	grub-mkrescue -o $@ $(BUILD)/iso/

test:$(BUILD)/img/h16M.img

.PHONY: clean
clean:
	rm $(BUILD)/boot -rf
	rm $(BUILD)/img -rf
	rm $(BUILD)/kernel -rf


.PHONY:bochs
bochs:$(BUILD)/img/h16M.img
	sudo bochs -f $(BUILD)/bochsrc -unlock

.PHONY:bochsb
bochsb:$(BUILD)/img/kernel.iso
	sudo bochs -f $(BUILD)/bochsrc.grub -unlock

QEMU:=sudo qemu-system-i386 \
			-m 32M \
			-rtc base=localtime \

QEMU_DISK:=	-boot c\
			-drive file=$(BUILD)/img/h16M.img,if=ide,index=0,media=disk,format=raw \

QEMU_CDROM:= -boot d\
			 -drive file=$(BUILD)/img/kernel.iso,media=cdrom \

QEMU_DEBUG:= -s -S

.PHONY:qemu
qemu:$(BUILD)/img/h16M.img
	$(QEMU) $(QEMU_DISK)

.PHONY:qemug
qemug:$(BUILD)/img/h16M.img
	$(QEMU) $(QEMU_DEBUG)

.PHONY:qemub
qemub:$(BUILD)/img/kernel.iso
	$(QEMU) $(QEMU_CDROM) 

$(BUILD)/img/h16M.vmdk:$(BUILD)/img/h16M.img
	sudo qemu-img convert -O vmdk $< $@

.PHONY:vmdk
vmdk:$(BUILD)/img/h16M.img