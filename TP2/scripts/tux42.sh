#!/bin/bash
ifconfig eth0 up
ifconfig eth0 172.16.41.1
route add -net 172.16.40.0/24 -gw 172.16.41.253
route -n
echo 1 > /proc/sys/net/ipv4/ip_forward
echo 0 > /proc/sys/net/ipv4/icmp_echo_ignore_broadcasts
