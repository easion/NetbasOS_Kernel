set $mdcr  = 0xffffc5
set $wscr  = 0xffffc7
set $p1ddr = 0xffffb0
set $p2ddr = 0xffffb1
set $paddr = 0xffffab

set remotebaud 9600
set radix
set radix 16
set machine h8300s
set remote X-packet 0
target remote /dev/ttyS0

set debug remote 1

set *(char*)$wscr = 0
set *(char*)$mdcr = 0x80
set *(char*)$p1ddr = 0xff
set *(char*)$p2ddr = 0xff
set *(char*)$paddr = 0xff



#target remote /dev/ttyS0
#load
#tb main
#c
#display/i $pc
#set debug remote 1

