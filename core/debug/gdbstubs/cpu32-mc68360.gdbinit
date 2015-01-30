#
# Target setup code for EDI, Inc.'s THUNDER platform
# Copyright (c) 2001 by EDI, Inc.  All Rights Reserved.
#
# Written by Bill Gatliff, bgat@billgatliff.com,
# with considerable help from Mike Dorin, dorin@gr-303.com
#


# if you change $dprbase, you'll have to change
# the corresponding settings in the stub
set $dprbase = 0xffff0000

set $regb = $dprbase + 0x1000

set $clkocr = $regb + 0xc
set $cr = $regb + 0x5c0
set $gmr = $regb + 0x40
set $padir = $regb + 0x550
set $papar = $regb + 0x552
set $padat = $regb + 0x556
set $pepar = $regb + 0x16
set $pllcr = $regb + 0x10
set $sdcr = $regb + 0x51e
set $sypcr = $regb + 0x22

set $br0 = $regb + 0x50
set $or0 = $regb + 0x54
set $br1 = $regb + 0x60
set $or1 = $regb + 0x64
set $br2 = $regb + 0x70
set $or2 = $regb + 0x74
set $br3 = $regb + 0x80
set $or3 = $regb + 0x84
set $br4 = $regb + 0x90
set $or4 = $regb + 0x94
set $br5 = $regb + 0xa0
set $or5 = $regb + 0xa4
set $br6 = $regb + 0xb0
set $or6 = $regb + 0xb4
set $br7 = $regb + 0xc0
set $or7 = $regb + 0xc4

# set up the PLL and clocks
set *(short*)$pllcr = 0xc000
set *(char*)$clkocr = 1

# disable the SWT
set *(char*)$sypcr = 0x0c

# twiddle with various internal registers
set *(short*)$sdcr = 0x740

# configure some port i/o lines
set *(short*)$pepar = 0x720
set *(short*)$papar = 0xea7f
set *(short*)$padir |= 0x1580
set *(short*)$padat |= 0x1580

# set up basic DRAM timing
set *(long*)$gmr = 0x149400a0

# program CS1: 16MB DRAM, 1 wait state, page mode
set *(long*)$or1 = 0x2e000009
set *(long*)$br1 = 0x1000001

# set up CS0: 8 bits wide, 4 wait states
set *(long*)$or0 = 0x5fe00004
set *(long*)$br0 = 1

# set up CS6: 8 bits wide, 15 wait states
set *(long*)($or6) = 0xfff80004
set *(long*)($br6) = 0x9000001

