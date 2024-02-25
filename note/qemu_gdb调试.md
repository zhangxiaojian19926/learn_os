# qemu 安装

```shell
# 下载源码
git clone git@github.com:qemu/qemu.git
```

```shell
# 安装 
sudo apt-get install libglib2.0-dev -y
sudo apt-get install libpixman-1-dev -y
sudo apt-get install libfdt-dev -y
sudo apt-get install ninja-build -y

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

### 安装qemu

```shell


sudo apt install qemu qemu-kvm libvirt-daemon libvirt-clients bridge-utils virt-manager

qemu-system-x86_64 --version

QEMU emulator version 2.11.1(Debian 1:2.11+dfsg-1ubuntu7.41)Copyright (c) 2003-2017 Fabrice Bellard and the QEMU Project developers

sudo apt-get update


sudo apt-get install qemu-system-arm


qemu-system-aarch64 -M virt -cpu help

Available CPUs: arm1026 arm1136 arm1136-r2 arm1176 arm11mpcore arm926 arm946 cortex-a15 cortex-a53 cortex-a57 cortex-a7 cortex-a8 cortex-a9 cortex-m3 cortex-m4 cortex-r5 pxa250 pxa255 pxa260 pxa261 pxa262 pxa270-a0 pxa270-a1 pxa270 pxa270-b0 pxa270-b1 pxa270-c0 pxa270-c5 sa1100 sa1110 ti925t
```
