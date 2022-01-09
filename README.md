##changes:

###added:

`gnash` (gnu-Flash up to v7) as an engine to build GUIs.
to coomunicate with outside-flash world added plugin `USocketIO` (a regular streaming Unix-socket).

to create:
```
var s = new USocketIO("");

var r = s.socket("path_name_of_sock");
// r is a result here.
```

to begin listen (server side):
```
s.listen();
```

to connect (client side):
```
s.connect();
```

to read/write:
```
var r = s.read(); // r is string with just read data or boolean false if none read

s.write("a string to write to the socket);
```

to close:
```
s.close();
```

read, write, close work with both server and client sockets.


defconfig:
```
make widora_mangopi_r3_gnash_defconfig
```

# Buildroot Package for Allwinner SIPs
Opensource development package for Allwinner F1C100s & F1C200s

## Driver support
Check this file to view current driver support progress for F1C100s/F1C200s: [PROGRESS-SUNIV.md](PROGRESS-SUNIV.md)

Check this file to view current driver support progress for V3/V3s/S3/S3L: [PROGRESS-V3.md](PROGRESS-V3.md)

## Install

### Install necessary packages
``` shell
sudo apt install wget unzip build-essential git bc swig libncurses-dev libpython3-dev libssl-dev
sudo apt install python3-distutils
```

### Download BSP
**Notice: Root permission is not necessery for download or extract.**
```shell
git clone https://github.com/mangopi-sbc/buildroot-mangopi-r
```

## Make the first build
**Notice: Root permission is not necessery for build firmware.**

### Apply defconfig
**Caution: Apply defconfig will reset all buildroot configurations to default values.**

**Generally, you only need to apply it once.**
```shell
cd buildroot-mangopi-r
make widora_mangopi_r3_defconfig
```

### Regular build
```shell
make
```

## Speed up build progress

### Download speed
Buildroot will download sourcecode when compiling the firmware. You can grab a **TRUSTWORTHY** archive of 'dl' folder for speed up.

### Compile speed
If you have a multicore CPU, you can try
```
make -j ${YOUR_CPU_COUNT}
```
or buy a powerful PC for yourself.

## Flashing firmware to target
You can flash a board by Linux (Recommended) or Windows system.
### [Here is the manual.](flashutils/README.md)

## Helper Scripts
- rebuild-uboot.sh: Recompile U-Boot when you direct edit U-Boot sourcecode.
- rebuild-kernel.sh: Recompile Kernel when you direct edit Kernel sourcecode.
- emulate-chroot.sh: Emulate target rootfs by chroot.
