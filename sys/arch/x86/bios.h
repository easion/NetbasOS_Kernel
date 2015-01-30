
#ifndef _BIOS_H
#define _BIOS_H

// register values on _bios_int() call and return
struct bios_regs {
	unsigned long eax __attr_packet;
	unsigned long ebx __attr_packet;
	unsigned long ecx __attr_packet;
	unsigned long edx __attr_packet;
	unsigned long esi __attr_packet;
	unsigned long edi __attr_packet;
	unsigned short es __attr_packet;
} ;

void bios_int(unsigned char bint, struct bios_regs* bregs);

#endif

