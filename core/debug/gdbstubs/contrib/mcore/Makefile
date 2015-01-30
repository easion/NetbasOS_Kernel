PREFIX=/opt/devsys/mcore

CC=${PREFIX}/bin/mcore-elf-gcc
CFLAGS=-Wall -Wstrict-prototypes -m210 -mbig-endian -c
LD=${PREFIX}/bin/mcore-elf-ld
LDFLAGS=-EB -L ${PREFIX}/mcore-elf/lib/big/m210 -static -lc -lm
OC=${PREFIX}/bin/mcore-elf-objcopy
OCFLAGS=-g --gap-fill 0xFF -O srec
OD=${PREFIX}/bin/mcore-elf-objdump
ODFLAGS=--wide --source --all-headers --reloc --syms --disassemble --endian=big

all :        all-stub all-bubble
all-stub :   stub-mmc2107 \
             stub-mmc2114 \
             stub-evb2107 \
             stub-evb2114 \
             stub-cme2107 \
             stub-nmin2107 \
             stub-nmin2114
all-bubble : bubble-evb2107 \
             bubble-evb2114 \
             bubble-cme2107 \
             bubble-nmin2107 \
             bubble-nmin2114 \
             bubble-mmc2107-ram \
             bubble-mmc2114-ram \
             bubble-evb2107-ram \
             bubble-evb2114-ram \
             bubble-evb2107-int \
             bubble-evb2114-int \
             bubble-evb2107-ext \
             bubble-evb2114-ext \
             bubble-cme2107-ram \
             bubble-cme2107-int 

vectors.o : vectors.c mcore.h
	${CC} ${CFLAGS} vectors.c -o vectors.o

sgfm_config.o : sgfm_config.c
	${CC} ${CFLAGS} sgfm_config.c -o sgfm_config.o

stub.o : stub.c mcore.h
	${CC} ${CFLAGS} stub.c -o stub.o

bubble.o : bubble.c
	${CC} ${CFLAGS} -g bubble.c -o bubble.o

gdb.o : gdb.c gdb.h
	${CC} ${CFLAGS} -DGDB_RXBUFLEN=300 -DNO_XPACKET -DNO_TPACKET gdb.c -o gdb.o

stub_obj = vectors.o mcore.o stub.o gdb.o sgfm_config.o
bubble_obj = mcore.o vectors.o gdb.o sgfm_config.o bubble.o 

#
# gdb stubs that reside in flash
#

# gdb stub for on-chip mmc2107 flash on EVB2107 (8MHz ref clock)
stub-mmc2107 : ${stub_obj}
	${CC} ${CFLAGS} -DSYNCR_MFD=0x2000 -DENABLE_SIGNAL_MSG mcore.c -o mcore.o
	${LD} -S -T stub-mcore.ld ${stub_obj} -o stub-mmc2107.elf ${LDFLAGS}
	${OC} ${OCFLAGS} stub-mmc2107.elf stub-mmc2107.s19
	${OD} ${ODFLAGS} stub-mmc2107.elf > stub-mmc2107.lst

# gdb stub for on-chip mmc2114 flash on EVB2114 (4MHz ref clock)
stub-mmc2114 : ${stub_obj}
	${CC} ${CFLAGS} -DSYNCR_MFD=0x6000 -DENABLE_SIGNAL_MSG mcore.c -o mcore.o
	${LD} -S -T stub-mcore.ld ${stub_obj} -o stub-mmc2114.elf ${LDFLAGS}
	${OC} ${OCFLAGS} stub-mmc2114.elf stub-mmc2114.s19
	${OD} ${ODFLAGS} stub-mmc2114.elf > stub-mmc2114.lst

# gdb stub for on-chip mmc2107 flash on Axiom CME-2107 (4MHz ref clock)
stub-cme2107 : ${stub_obj}
	${CC} ${CFLAGS} -DSYNCR_MFD=0x6000 -DENABLE_SIGNAL_MSG mcore.c -o mcore.o
	${LD} -S -T stub-mcore.ld ${stub_obj} -o stub-cme2107.elf ${LDFLAGS}
	${OC} ${OCFLAGS} stub-cme2107.elf stub-cme2107.s19
	${OD} ${ODFLAGS} stub-cme2107.elf > stub-cme2107.lst

# gdb stub for on-chip mmc2107 flash on New Micros NMIN-2107 (8MHz ref clock)
stub-nmin2107 : ${stub_obj}
	${CC} ${CFLAGS} -DSYNCR_MFD=0x2000 -DENABLE_SIGNAL_MSG mcore.c -o mcore.o
	${LD} -S -T stub-nmin.ld ${stub_obj} -o stub-nmin2107.elf ${LDFLAGS}
	${OC} ${OCFLAGS} stub-nmin2107.elf stub-nmin2107.s19
	${OD} ${ODFLAGS} stub-nmin2107.elf > stub-nmin2107.lst

# gdb stub for on-chip mmc2114 flash on New Micros NMIN-2114 (4MHz ref clock)
stub-nmin2114 : ${stub_obj}
	${CC} ${CFLAGS} -DSYNCR_MFD=0x6000 -DENABLE_SIGNAL_MSG mcore.c -o mcore.o
	${LD} -S -T stub-nmin.ld ${stub_obj} -o stub-nmin2114.elf ${LDFLAGS}
	${OC} ${OCFLAGS} stub-nmin2114.elf stub-nmin2114.s19
	${OD} ${ODFLAGS} stub-nmin2114.elf > stub-nmin2114.lst

# gdb stub for external AMD flash on EVB2107 (8MHz ref clock)
stub-evb2107 : ${stub_obj}
	${CC} ${CFLAGS} -DCSCR2=0x2303 -DSYNCR_MFD=0x2000 -DENABLE_SIGNAL_MSG mcore.c -o mcore.o
	${LD} -S -T stub-evb.ld ${stub_obj} -o stub-evb2107.elf ${LDFLAGS}
	${OC} ${OCFLAGS} stub-evb2107.elf stub-evb2107.s19
	${OD} ${ODFLAGS} stub-evb2107.elf > stub-evb2107.lst

# gdb stub for external AMD flash on EVB2114 (4MHz ref clock)
stub-evb2114 : ${stub_obj}
	${CC} ${CFLAGS} -DCSCR2=0x2303 -DSYNCR_MFD=0x6000 -DENABLE_SIGNAL_MSG mcore.c -o mcore.o
	${LD} -S -T stub-evb.ld ${stub_obj} -o stub-evb2114.elf ${LDFLAGS}
	${OC} ${OCFLAGS} stub-evb2114.elf stub-evb2114.s19
	${OD} ${ODFLAGS} stub-evb2114.elf > stub-evb2114.lst

#
# RAM-based projects needing a resident stub to launch them
#

# sample project for internal mmc2107 RAM (single-chip mode)
bubble-mmc2107-ram : bubble.o
	${LD} -T mcore-ram.ld bubble.o -o bubble-mmc2107-ram.elf ${LDFLAGS}
	${OC} -O srec bubble-mmc2107-ram.elf bubble-mmc2107-ram.s19
	${OD} ${ODFLAGS} bubble-mmc2107-ram.elf > bubble-mmc2107-ram.lst

# sample project for internal mmc2114 RAM (single-chip mode)
bubble-mmc2114-ram : bubble.o
	${LD} -T mcore-ram.ld bubble.o -o bubble-mmc2114-ram.elf ${LDFLAGS}
	${OC} -O srec bubble-mmc2114-ram.elf bubble-mmc2114-ram.s19
	${OD} ${ODFLAGS} bubble-mmc2114-ram.elf > bubble-mmc2114-ram.lst

# sample project for external RAM on the EVB/CMB2107
bubble-evb2107-ram : vectors.o gdb.o sgfm_config.o bubble.o
	${CC} ${CFLAGS} -DSYNCR_MFD=0x2000 -DENABLE_SIGNAL_MSG mcore.c -o mcore.o
	${LD} -T evb-ram.ld ${bubble_obj} -o bubble-evb2107-ram.elf ${LDFLAGS}
	${OC} ${OCFLAGS} bubble-evb2107-ram.elf bubble-evb2107-ram.s19
	${OD} ${ODFLAGS} bubble-evb2107-ram.elf > bubble-evb2107-ram.lst

# sample project for external RAM on the EVB/CMB2114
bubble-evb2114-ram : vectors.o gdb.o sgfm_config.o bubble.o
	${CC} ${CFLAGS} -DSYNCR_MFD=0x6000 -DENABLE_SIGNAL_MSG mcore.c -o mcore.o
	${LD} -T evb-ram.ld ${bubble_obj} -o bubble-evb2114-ram.elf ${LDFLAGS}
	${OC} ${OCFLAGS} bubble-evb2114-ram.elf bubble-evb2114-ram.s19
	${OD} ${ODFLAGS} bubble-evb2114-ram.elf > bubble-evb2114-ram.lst

# sample project for external RAM on the Axiom CME-2107
bubble-cme2107-ram : vectors.o gdb.o sgfm_config.o bubble.o
	${CC} ${CFLAGS} -DSYNCR_MFD=0x6000 -DENABLE_SIGNAL_MSG mcore.c -o mcore.o
	${LD} -T cme-ram.ld ${bubble_obj} -o bubble-cme2107-ram.elf ${LDFLAGS}
	${OC} ${OCFLAGS} bubble-cme2107-ram.elf bubble-cme2107-ram.s19
	${OD} ${ODFLAGS} bubble-cme2107-ram.elf > bubble-cme2107-ram.lst

#
# Stand-alone projects that reside in flash and will run from reset
#

# standalone project for single-chip mode (internal flash / internal RAM) on EVB/CMB2107
bubble-evb2107 : vectors.o gdb.o sgfm_config.o bubble.o
	${CC} ${CFLAGS} -DSYNCR_MFD=0x2000 -DENABLE_SIGNAL_MSG mcore.c -o mcore.o
	${LD} -T evb-mcore.ld ${bubble_obj} -o bubble-evb2107.elf ${LDFLAGS}
	${OC} ${OCFLAGS} bubble-evb2107.elf bubble-evb2107.s19
	${OD} ${ODFLAGS} bubble-evb2107.elf > bubble-evb2107.lst

# standalone project for single-chip mode (internal flash / internal RAM) on EVB/CMB2114
bubble-evb2114 : vectors.o gdb.o sgfm_config.o bubble.o
	${CC} ${CFLAGS} -DSYNCR_MFD=0x6000 -DENABLE_SIGNAL_MSG mcore.c -o mcore.o
	${LD} -T evb-mcore.ld ${bubble_obj} -o bubble-evb2114.elf ${LDFLAGS}
	${OC} ${OCFLAGS} bubble-evb2114.elf bubble-evb2114.s19
	${OD} ${ODFLAGS} bubble-evb2114.elf > bubble-evb2114.lst

# standalone project for internal flash / external RAM on EVB/CMB2107
bubble-evb2107-int : vectors.o gdb.o sgfm_config.o bubble.o
	${CC} ${CFLAGS} -DCSCR2=0x2303 -DSYNCR_MFD=0x2000 -DENABLE_SIGNAL_MSG mcore.c -o mcore.o
	${LD} -T evb-int.ld ${bubble_obj} -o bubble-evb2107-int.elf ${LDFLAGS}
	${OC} ${OCFLAGS} bubble-evb2107-int.elf bubble-evb2107-int.s19
	${OD} ${ODFLAGS} bubble-evb2107-int.elf > bubble-evb2107-int.lst

# standalone project for external flash / external RAM on the EVB/CMB2107
bubble-evb2107-ext : vectors.o gdb.o sgfm_config.o bubble.o
	${CC} ${CFLAGS} -DCSCR2=0x2303 -DSYNCR_MFD=0x2000 -DENABLE_SIGNAL_MSG mcore.c -o mcore.o
	${LD} -T evb-ext.ld ${bubble_obj} -o bubble-evb2107-ext.elf ${LDFLAGS}
	${OC} ${OCFLAGS} bubble-evb2107-ext.elf bubble-evb2107-ext.s19
	${OD} ${ODFLAGS} bubble-evb2107-ext.elf > bubble-evb2107-ext.lst

# standalone project for internal flash / external RAM on EVB/CMB2114
bubble-evb2114-int : vectors.o gdb.o sgfm_config.o bubble.o
	${CC} ${CFLAGS} -DCSCR2=0x2303 -DSYNCR_MFD=0x6000 -DENABLE_SIGNAL_MSG mcore.c -o mcore.o
	${LD} -T evb-int.ld ${bubble_obj} -o bubble-evb2114-int.elf ${LDFLAGS}
	${OC} ${OCFLAGS} bubble-evb2114-int.elf bubble-evb2114-int.s19
	${OD} ${ODFLAGS} bubble-evb2114-int.elf > bubble-evb2114-int.lst

# standalone project for external flash / external RAM on the EVB/CMB2114
bubble-evb2114-ext : vectors.o gdb.o sgfm_config.o bubble.o
	${CC} ${CFLAGS} -DCSCR2=0x2303 -DSYNCR_MFD=0x6000 -DENABLE_SIGNAL_MSG mcore.c -o mcore.o
	${LD} -T evb-ext.ld ${bubble_obj} -o bubble-evb2114-ext.elf ${LDFLAGS}
	${OC} ${OCFLAGS} bubble-evb2114-ext.elf bubble-evb2114-ext.s19
	${OD} ${ODFLAGS} bubble-evb2114-ext.elf > bubble-evb2114-ext.lst

# standalone project for internal flash / external RAM on Axiom CME-2107
bubble-cme2107-int : vectors.o gdb.o sgfm_config.o bubble.o
	${CC} ${CFLAGS} -DCSCR1=0x0303 -DSYNCR_MFD=0x6000 -DENABLE_SIGNAL_MSG mcore.c -o mcore.o
	${LD} -T cme-flash.ld ${bubble_obj} -o bubble-cme2107-int.elf ${LDFLAGS}
	${OC} ${OCFLAGS} bubble-cme2107-int.elf bubble-cme2107-int.s19
	${OD} ${ODFLAGS} bubble-cme2107-int.elf > bubble-cme2107-int.lst

# standalone project for single-chip mode on Axiom CME-2107
bubble-cme2107 : vectors.o gdb.o sgfm_config.o bubble.o
	${CC} ${CFLAGS} -DSYNCR_MFD=0x6000 -DENABLE_SIGNAL_MSG mcore.c -o mcore.o
	${LD} -T evb-mcore.ld ${bubble_obj} -o bubble-cme2107.elf ${LDFLAGS}
	${OC} ${OCFLAGS} bubble-cme2107.elf bubble-cme2107.s19
	${OD} ${ODFLAGS} bubble-cme2107.elf > bubble-cme2107.lst

# standalone project for single-chip mode on New Micros NMIN-2107
bubble-nmin2107 : vectors.o gdb.o sgfm_config.o bubble.o
	${CC} ${CFLAGS} -DSYNCR_MFD=0x2000 -DENABLE_SIGNAL_MSG mcore.c -o mcore.o
	${LD} -T nmin-mcore.ld ${bubble_obj} -o bubble-nmin2107.elf ${LDFLAGS}
	${OC} ${OCFLAGS} bubble-nmin2107.elf bubble-nmin2107.s19
	${OD} ${ODFLAGS} bubble-nmin2107.elf > bubble-nmin2107.lst

# standalone project for single-chip mode on New Micros NMIN-2114
bubble-nmin2114 : vectors.o gdb.o sgfm_config.o bubble.o
	${CC} ${CFLAGS} -DSYNCR_MFD=0x6000 -DENABLE_SIGNAL_MSG mcore.c -o mcore.o
	${LD} -T nmin-mcore.ld ${bubble_obj} -o bubble-nmin2114.elf ${LDFLAGS}
	${OC} ${OCFLAGS} bubble-nmin2114.elf bubble-nmin2114.s19
	${OD} ${ODFLAGS} bubble-nmin2114.elf > bubble-nmin2114.lst

#####

.PHONY : clean
clean :
	rm -vf *.elf *.lst *.o *.s19
