#!/bin/bash
echo "Building the PRU code"
pasm -b pru_ads1256.p

echo "Building the Host application"
gcc host_ads1256.c -o host_ads1256 -lprussdrv
