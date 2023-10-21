BUILD:=.
SRC:=../src
ROOT:=.

ENTRYPOINT:=0x10000 #内核起始位置

# 创建boot.bin文件
$(BUILD)/boot/%.bin: $(SRC)/boot/%.asm
	$(shell mkdir -p $(dir $@))
	nasm -f bin $< -o $@

# 创建kernel文件
$(BUILD)/kernel/%.o: $(SRC)/kernel/%.asm
	$(shell mkdir -p $(dir $@))
	nasm -f elf32 $< -o $@

# 创建kernel.bin文件
$(BUILD)/kernel/kernel.bin: $(BUILD)/kernel/start.o
	$(shell mkdir -p $(dir $@))
	# $<表示所有以来文件 $@  目标文件，静态链接elf_i386的可执行文件
	ld -m elf_i386 -static $< -o $@ -Ttext $(ENTRYPOINT) 

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
	cd $(BUILD)/img && echo "1\nhd\nflat\n512\n16\nh16M.img" | sudo bximage # 加上&& 才可以进入其他目录执行
	sudo dd if=$(BUILD)/boot/boot.bin of=$@ bs=512 count=1 conv=notrunc
	sudo dd if=$(BUILD)/boot/loader.bin of=$@ bs=512 count=4 seek=2 conv=notrunc 
	sudo dd if=$(BUILD)/kernel/system.bin of=$@ bs=512 count=200 seek=10 conv=notrunc

test:$(BUILD)/img/h16M.img

.PHONY: clean
clean:
	rm $(BUILD)/boot -rf
	rm $(BUILD)/img -rf
	rm $(BUILD)/kernel -rf


.PHONY:bochs
bochs:$(BUILD)/img/h16M.img
	sudo bochs -f $(BUILD)/bochsrc
