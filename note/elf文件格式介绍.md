# ELF文件简单介绍

executable and linking format 可执行、可链接的格式

1. 可执行程序  gcc
2. 可重定位文件  gcc -c *.o / ar *.a
3. 共享目标文件，动态链接库  .so

## 可执行程序

1. 代码  .txt段 section ELF / 很多section可以组成segment cpu概念
2. 数据
   1. .data section / 已经初始化过的数据
   2. .bss 未初始化的数据，Block Start by Symbol


- https://refspecs.linuxfoundation.org/elf/elf.pdf