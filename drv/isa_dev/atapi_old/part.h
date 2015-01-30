#ifndef FSPART_H
#define FSPART_H

struct partitiontypes {  //////////����������
    unsigned char type;
    char *name;
} partition_types[] = {
    {0, "Unknow Partition"},
    {1, "DOS 12-bit FAT"},		/* Primary DOS with 12-bit FAT */
    {2, "XENIX /"},			/* XENIX / filesystem */
    {3, "XENIX /usr"},			/* XENIX /usr filesystem */
    {4, "DOS 16-bit FAT <32M"},		/* Primary DOS with 16-bit FAT */
    {5, "DOS Extended"},		/* DOS 3.3+ extended partition */
    {6, "DOS 16-bit FAT >=32M"},
    {7, "OS/2 IFS (e.g., HPFS) or NTFS or QNX2 or Advanced UNIX"},
    {8, "AIX boot or SplitDrive"},
    {9, "AIX data or Coherent"},
    {0x0a, "OS/2 Boot Manager or Coherent swap"},
    {0x0b, "Microsoft Windows FAT32"},
    {0x0c, "Windows FAT32 (lba)"},
    {0x0d, "Windows FAT16(lba)"},
    {0x0e, "MSDOS FAT16. CHS-mapped"},
    {0x0f, "Ext. partition, CHS-mapped"},
    {0x10, "OPUS"},
    {0x11, "OS/2 BM: hidden DOS 12-bit FAT"},
    {0x12, "Compaq diagnostics"},
    {0x14, "OS/2 BM: hidden DOS 16-bit FAT <32M"},
    {0x16, "OS/2 BM: hidden DOS 16-bit FAT >=32M"},
    {0x17, "OS/2 BM: hidden IFS"},
    {0x18, "AST Windows swapfile"},
    {0x24, "NEC DOS"},
    {0x3c, "PartitionMagic recovery"},
    {0x40, "Venix 80286"},
    {0x41, "Linux/MINIX (sharing disk with DRDOS)"},
    {0x42, "SFS or Linux swap (sharing disk with DRDOS)"},
    {0x43, "Linux native (sharing disk with DRDOS)"},
    {0x50, "DM (disk manager)"},
    {0x51, "DM6 Aux1 (or Novell)"},
    {0x52, "CP/M or Microport SysV/AT"},
    {0x53, "DM6 Aux3"},
    {0x54, "DM6"},
    {0x55, "EZ-Drive (disk manager)"},
    {0x56, "Golden Bow (disk manager)"},
    {0x5c, "Priam Edisk (disk manager)"}, /* according to S. Widlake */
    {0x61, "SpeedStor"},
    {0x63, "GNU HURD or Mach or Sys V/386 (such as ISC UNIX)"},
    {0x64, "Novell Netware 286"},
    {0x65, "Novell Netware 386"},
    {0x70, "DiskSecure Multi-Boot"},
    {0x75, "PC/IX"},
    {0x77, "QNX4.x"},
    {0x78, "QNX4.x 2nd part"},
    {0x79, "QNX4.x 3rd part"},
    {0x80, "MINIX until 1.4a"},
    {0x81, " MINIX Version 2. "},
    {0x82, "Unite Journal File System  Jicama native"},
    {0x83, "Linux native:Ext2 Fs"},
    {0x84, "OS/2 hidden C: drive"},
    {0x85, "Linux extended"},
    {0x86, "NTFS volume set??"},
    {0x87, "NTFS volume set??"},
    {0x90, "???????"},
    {0x93, "Amoeba"},
    {0x94, "Amoeba BBT"},		/* (bad block table) */
    {0xa0, "IBM Thinkpad hibernation"}, /* according to dan@fch.wimsey.bc.ca */
    {0xa5, "BSD/386"},			/* 386BSD */
    {0xa7, "NeXTSTEP 486"},
    {0xb7, "BSDI fs"},
    {0xb8, "BSDI swap"},
    {0xc1, "DRDOS/sec (FAT-12)"},
    {0xc4, "DRDOS/sec (FAT-16, < 32M)"},
    {0xc6, "DRDOS/sec (FAT-16, >= 32M)"},
    {0xc7, "Syrinx"},
    {0xdb, "CP/M or Concurrent CP/M or Concurrent DOS or CTOS"},
    {0xe1, "DOS access or SpeedStor 12-bit FAT extended partition"},
    {0xe3, "DOS R/O or SpeedStor"},
    {0xe4, "SpeedStor 16-bit FAT extended partition < 1024 cyl."},
    {0xf1, "SpeedStor"},
    {0xf2, "DOS 3.3+ secondary"},
    {0xf4, "SpeedStor large partition"},
    {0xfe, "SpeedStor >1024 cyl. or LANstep"},
    {0xff, "Xenix Bad Block Table"}
};
#endif


