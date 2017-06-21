#!/bin/sh
modprobe ip_tunnel
modprobe gre
insmod /lib/modules/4.4.15/kernel/net/ipv4/ip_gre.ko
lsmod |grep ip_gre
