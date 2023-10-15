# 创建boot.bin文件
boot.bin: boot.asm
	nasm -f bin boot.asm -o boot.bin

# 创建镜像文件
h16M.img: boot.bin
	echo "1\nhd\nflat\n512\n16\nh16M.img" | sudo bximage
	sudo dd if=boot.bin of=h16M.img bs=512 count=1 conv=notrunc

clean:
	rm boot.bin
	yes | rm h16M.img


.PHONY:bochs
bochs:h16M.img
	sudo bochs -f bochsrc
