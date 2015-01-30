/*
**     (R)Jicama OS
**     BIOS  Equip Implement
**     Copyright (C) 2003 DengPingPing
*/

#define EQAD ((0x040<<4)+0x010)		/* The BIOS equipment info	*/

struct equip_list			/* The equipment in the computer*/
{
   char Floppies;			/* Number of floppy disk drives	*/
   char HasCoprocessor;			/* Do we have an FPU ?		*/
   union
   {
      char MemoryBanks;			/* 16k memory banks in PCs	*/
      char HasMouse;			/* PS/2 mouse available (IBMs)  */
   } mix;
   char InitialVideoMode;		/* Video mode at boot		*/
   char HasDMASupport;			/* Do we support DMA ?		*/
   char SerialPorts;			/* How many serial ports ?	*/
   char HasGameport;			/* Do we have a gameport ?	*/
   char HasInternalModem;		/* Do we have an internal modem */
   char PrinterPorts;			/* How many printer ports ?	*/
};
struct equip_list *equip;

void bios_equip_list(char isIBMPS)
{
   /*
      Background info:
      The equipment in a computer is represented by one dword in the BIOS
      data area, at 0x040:0x010.  We take this and convert it to our own
      equipment list format.  Note that you cannot do a BIOS call to fetch
      the info (int 0x11) because we are in protected mode !!!
      EQAD is the absolute address of the BIOS equipment data dword
   */

   unsigned long BIOS_equip = *((unsigned long *)EQAD);	/* The BIOS equipment info	*/

   /* Check for floppies - this is bit 0 of the equipment dword		*/
   /* Bits 6+7 contain the amount of floppy drives in the system	*/

   if(BIOS_equip & 0x1)
   {
      equip->Floppies = ((BIOS_equip & 0x0c0) >> 6) + 1; /* How many ?	*/
   }
   else equip->Floppies = 0;

   /* Is there a coprocessor ?  This is bit 1				*/

   equip->HasCoprocessor = (BIOS_equip & 0x02) != 0;

   /* Now we make a distinction between an IBM PS/1 or PS/2 computer	*/
   /* (maybe even Aptiva ?) and a _real_ PC ;)				*/
   /* IBMs store the availability of a mouse here, PCs the RAM banks	*/
   /* Get bits 2 and 3							*/
   if(isIBMPS)
   {
      equip->mix.HasMouse = (BIOS_equip & 0x04) != 0;
   }
   else equip->mix.MemoryBanks = (BIOS_equip & 0x0c) >> 2;

   /* Get the initial video mode (video mode at boot -- bit 4 and 5)	*/
   equip->InitialVideoMode = (BIOS_equip & 0x030) >> 4;

   /* Does this computer support DMA transfers ?  (sure hope so !!!)	*/
   /* This is bit 8							*/
   equip->HasDMASupport = (BIOS_equip & 0x0100) != 0;

   /* Now we get the amount of RS232 serial ports (bits 9-11)	 	*/
   equip->SerialPorts = (BIOS_equip & 0x0e00) >> 9;

   /* Does this computer have a gameport ?  (bit 12)			*/
   equip->HasGameport = (BIOS_equip & 0x01000) != 0;

   /* Does this computer have an internal modem ? (bit 13)		*/
    equip->HasInternalModem = (BIOS_equip & 0x02000) != 0;

   /* How many printer ports do we have ?				*/
    equip->PrinterPorts = (BIOS_equip & 0x0c000) >> 14;

  #if 0
  switch (equip->InitialVideoMode)
  {
      case 0:
         kprintf("EGA, VGA or PGA\n");
         break;
      case 1:
         kprintf("40x25 color\n");
         break;
      case 2:
         kprintf("80x25 color\n");
         break;
      case 3:
         kprintf("80x25 monochrome\n");
         break;
      default : 
		  kprintf("Unknow video mode...  Weird BIOS ?\n");
	      break;
  }
  kprintf("BIOS: DMA Support    -- %s\n",equip->HasDMASupport?"Supported":"Not supported");
  kprintf("BIOS: Serial ports   -- %d\n",equip->SerialPorts);
  kprintf("BIOS: Game port      -- %s\n",equip->HasGameport?"Available":"Not available");
  kprintf("BIOS: Internal Modem -- %s\n",equip->HasInternalModem?"Available":"Not available");
  kprintf("BIOS: Printer ports  -- %d\n",equip->PrinterPorts);
   #endif
}
