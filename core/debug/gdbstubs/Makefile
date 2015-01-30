H8300_H8S2148EDK_STUB = main.c gdb.c h8300.c h8300.S
H8300_H8S2148EDK_TEST = test.c
H8300_PREFIX = h8300-elf-

CFLAGS = -Wall -g -O2

.PHONY: h8300-stub


h8300-stub: h8300-h8s2148edk-stub.elf h8300-h8s2148edk-stub.mot

h8300-test: h8300-h8s2148edk-test.elf

h8300-h8s2148edk-stub.mot: h8300-h8s2148edk-stub.elf
	$(H8300_PREFIX)objcopy -O srec $< $@

h8300-h8s2148edk-stub.elf: $(H8300_H8S2148EDK_STUB)
	$(H8300_PREFIX)gcc $(CFLAGS) -ms -mint32 -o $@ -nostartfiles -nodefaultlibs \
		-DCRT0 -DH8300_H8S2148EDK $(H8300_H8S2148EDK_STUB) \
		-Wl,--script=h8300-h8s2148edk-stub.x 
	$(H8300_PREFIX)objdump --syms $@ | grep " _start"

h8300-h8s2148edk-test.elf: $(H8300_H8S2148EDK_TEST)
	$(H8300_PREFIX)gcc $(CFLAGS) -ms -mint32 -o $@ $(H8300_H8S2148EDK_TEST) \
		-Wl,--script=h8300-h8s2148edk.x 

clean:
	rm -f *.mot *.elf
