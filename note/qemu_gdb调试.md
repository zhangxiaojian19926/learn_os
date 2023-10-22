# qemu 安装
```shell
# 下载源码
git clone git@github.com:qemu/qemu.git
```

```shell
# 安装
sudo apt-get install libglib2.0-dev
sudo apt-get install libpixman-1-dev
sudo apt-get install libfdt-dev
sudo apt-get install ninja-build

mkdir build
cd build
../configure
make -j16
```

```shell
#运行镜像命令
qemu-system-i386 \
-m 32M \
-boot c\
-hda $<

# 运行GDB调试
qemu-system-i386 \
    -s -S  \
    -m 32M \
    -boot c\
    -hda $<
```