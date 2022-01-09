#!/bin/sh

sunxi-fel -p uboot output/images/u-boot-sunxi-with-spl.bin

#sleep 3

dfu-util -R -a all -D output/images/sysimage-nand.img
