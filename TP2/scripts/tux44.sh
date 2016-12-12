#!/bin/bash
/etc/init.d/networking restart
ifconfig eth0 up
ifconfig eth0 172.16.40.254/24
ifconfig eth1 up
ifconfig eth1 172.16.41.253/24
route add default gw 172.16.41.254
echo 1 > /proc/sys/net/ipv4/ip_forward
echo 0 > /proc/sys/net/ipv4/icmp_echo_ignore_broadcasts
printf "search lixa.netlab.fe.up.pt\nnameserver 172.16.1.1\n" > /etc/resolv.conf
