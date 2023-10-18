# 创建boot.bin文件
%.bin: %.asm
	nasm -f bin $< -o $@

# 创建镜像文件，boot从第1个扇区开始写，loader从第2个扇区开始写，到第4个扇区为止  
h16M.img: boot.bin loader.bin
	echo "1\nhd\nflat\n512\n16\nh16M.img" | sudo bximage
	sudo dd if=boot.bin of=h16M.img bs=512 count=1 conv=notrunc
	sudo dd if=loader.bin of=h16M.img bs=512 count=4 seek=2 conv=notrunc 

.PHONY: clean
clean:
	rm *.bin
	yes | rm *.img
	yes | rm *.img.lock


.PHONY:bochs
bochs:h16M.img
	sudo bochs -f bochsrc
