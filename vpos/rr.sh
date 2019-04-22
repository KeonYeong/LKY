#!/bin/bash
source /etc/profile
make clean
make
cp images/vpos.bin /tftpboot
