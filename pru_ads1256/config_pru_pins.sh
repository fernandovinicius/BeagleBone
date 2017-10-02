#!/bin/bash
echo "Configuring PRU Memory"
rmmod uio_pruss
modprobe uio_pruss extram_pool_sz=0x1E8480

echo "Checking pins"
cat /sys/kernel/debug/pinctrl/44e10800.pinmux/pins | grep '107\|103\|105\|101\|102'

echo "Compiling the overlay from .dts to .dtbo"
dtc -O dtb -o PRU-SPI-ADS1256-00A0.dtbo -b 0 -@ PRU-SPI-ADS1256.dts

echo "Obs1: Copy overlay file (.dtbo) to '/lib/firmware'"
echo "Obs2: Comment enable-universal-cape line on '/boot/uEnv.txt' file"
echo "Obs3: Add CAPE=PRU-SPI-ADS1256 to '/etc/default/capemgr' file"
