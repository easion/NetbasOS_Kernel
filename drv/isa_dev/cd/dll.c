
/* 
** Jicama OS Loadable Kernel Modules Test
** 2005-3-5
*/
#include <drv/drv.h>
 int init();

/*dll entry*/
int dll_main(char **argv)
{
	 init();

	return 0;
}


int dll_destroy()
{
	printk("dll_destroy called!\n");
	return 0;
}

int dll_version()
{
	printk("VERSION 0.01!\n");
	return 0;
}
/*****************************************************************************
	name:	atapiRipAudio

	NOT TESTED -- my drive does not support READ CD

44100 samples/sec * 16 bits/sample * 2 (stereo) = 1 411 200 bits/sec

1 411 200 bits/sec * 1/75 sec/frame = 18 816 bits/frame

18 816 bits/frame / 8 bits/byte = 2352 bytes/frame
*****************************************************************************/
/* read a great big wad of data to test */
//#define	WAD_SIZE	41289u	/* not divisible by 512 nor 2048 */
//#define	WAD_SIZE	47616u	/* divisible by 512 but not 2048 */
//#define	WAD_SIZE	47040u	/* 20 audio frames, 1/75 sec each */

#define	WAD_SIZE	49152u	/* divisible by 512 and 2048 */

static u8_t GlobalBuffer[WAD_SIZE];

#if 0

static int atapiRipAudio(driveinfo *Drive)//, atapimsf *Start, atapimsf *End)
{//	static u8 GlobalBuffer[FOO];
	u8 Pkt[12]={
		0xBE, /* READ CD */
		0,
		0, 0, 0, 0,	/* starting LBA */
		0, 0, 0,	/* block count */
		0x10,		/* only user data */
		0,		/* no subchannel data */
		0 };
	drivecmd Cmd;
	FILE *Outfile;

	DEBUG(printk("atapiRipAudio\n");)
	Cmd.Blk=0;
	Cmd.Count=WAD_SIZE;
	Cmd.Cmd=0;
	Cmd.Dev=0;
	Cmd.Data=GlobalBuffer;

/* 20 frames, 1/75 sec = only 1/4 second of audio
If you compile with DJGPP, you can make WAD_SIZE much bigger. */
	Pkt[8]=WAD_SIZE / 2352;
	if(atapiCmd2(Drive, &Cmd, Pkt) != 0)
		return(-1);
	Outfile=fopen("foo", "wb");
	fwrite(GlobalBuffer, 1, WAD_SIZE, Outfile);
	fclose(Outfile);
	return(0); }



0	1 0 1 1 1 1 1 0		opcode (BEh, "READ CD")
1	0 0 0 t t t 0 0		t=expected sector (0=any),
2	MSB			starting LBA
3	-			' '
4	-			' '
5	LSB			' '
6	MSB			transfer length (blocks)
7	-			' '
8	LSB			' '
9	s-h-h-u-ecc-e-e-0	s=include sync field,
				h=include header and subheader,
				u=include user data, (=1)
				ecc=include ECC data,
				e=include error flag data,
10	0 0 0 0 0 b b b		b=include subchannel
11	0 0 0 0 0 0 0 0		reserved

#endif
/*****************************************************************************
	name:	demoDataCD
*****************************************************************************/
#if 0
static int demoDataCD(driveinfo *Drive)
{	//u8 GlobalBuffer[ATAPI_SECTSIZE];
	drivecmd Cmd;
	int Temp;

	if(Drive->IOAdr == 0 || !Drive->IsATAPI)
		return(-1);
	printk("trying to read from ATAPI CD-ROM\n");
/* read random sectors */
	Cmd.Cmd=DRV_CMD_RD;
//	Cmd.Dev=0;
	Cmd.Data=GlobalBuffer;
	Cmd.Blk=rand();
	Drive->UseLBA=1;
	Cmd.Count=WAD_SIZE;
	Temp=atapiCmd(Drive, &Cmd);
	if(Temp != 0)
	{	printk("\n");
		return(Temp); }
/* read sector 16 of CD-ROM (root directory) */
	Cmd.Cmd=DRV_CMD_RD;
//	Cmd.Dev=0;
	Cmd.Data=GlobalBuffer;
	Cmd.Blk=16;
	Drive->UseLBA=1;
	Cmd.Count=WAD_SIZE;
	Temp=atapiCmd(Drive, &Cmd);
	if(Temp != 0)
	{	printk("\n");
		return(Temp); }
/* dump sector 16 */
	dump(GlobalBuffer, 96);
	return(0); }
/*****************************************************************************
	name:	demoAudioCD
*****************************************************************************/
static int demoAudioCD(driveinfo *Drive)
{	bool Load=FALSE, Continue=FALSE;
	int Temp, Tries, CurrTrack=0, Key;

	if(Drive->IOAdr == 0 || !Drive->IsATAPI)
		return(-1);
	for(Tries=3; Tries != 0; Tries--)
	{	printk("trying to read audio CD table-of-contents\n");
		Temp=atapiTOC(Drive);
		if(Temp == 0)
			break;
		delay(1000); }
	if(Temp != 0)
		return(Temp);
	printk("I think it is an audio CD\n");
	goto HELP;

	while(1)
	{	Key=getch();
		if(Key == 0)
			Key=0x100 | getch();
/* H or h or ? == help */
		if(Key == 'h' || Key == 'H' || Key == '?')
HELP:		{	printk("  Esc or Q to quit (esp. if not audio CD)\n"
				"  E to load/eject\n"
				"  T to load table-of-contents\n"
				"  right and left arrows to set current track\n"
				"  P to play current track\n"
//				"  R to \"rip\" 1/4 sec. audio to file 'foo'\n"
				"  S to pause/continue\n\n"); }
/* Esc or Q == quit */
		else if(Key == 27 || Key == 'q' || Key == 'Q')
			break;
/* E == load/eject */
		else if(Key == 'e' || Key == 'E')
		{	if(Load)
				printk("LOAD\n");
			else printk("EJECT\n");
			atapiEject(Drive, Load);
			Load=!Load; }
/* S == pause/continue */
		else if(Key == 's' || Key == 'S')
		{	if(Continue)
				printk("CONTINUE\n");
			else printk("PAUSE\n");
			atapiPause(Drive, Continue);
			Continue=!Continue; }
/* T == load table-of-contents */
		else if(Key == 't' || Key == 'T')
		{	printk("LOAD TABLE-OF-CONTENTS\n");
			atapiTOC(Drive);
			CurrTrack=0; }
/* P == play current track */
		else if(Key == 'p' || Key == 'P')
		{	atapimsf *Start, *End;

			Start=Track + CurrTrack;
			End=Track + (CurrTrack + 1);
			printk("PLAY TRACK %u (%02u:%02u:%02u to "
				"%02u:%02u:%02u)\n", CurrTrack + 1,
				Start->Min, Start->Sec, Start->Frame,
				End->Min, End->Sec, End->Frame);
			atapiPlay(Drive, Start, End); }
/* right arrow == next track */
		else if(Key == 0x14D)
		{	if(CurrTrack < NumTracks - 1)
				CurrTrack++;
			printk("NEXT TRACK (%u)\n",
				CurrTrack + 1); }
/* left arrow == previous track */
		else if(Key == 0x14B)
		{	if(CurrTrack > 0)
				CurrTrack--;
			printk("PREVIOUS TRACK (%u)\n",
				CurrTrack + 1); }
/* R to rip audio */
//		else if(Key == 'r' || Key == 'R')
/*		{	atapiRipAudio(Drive); }*/}
	return(0); }
/*****************************************************************************
	name:	main
*****************************************************************************/
int main(void)
{	vector Vector14, Vector15;
	unsigned WhichDrive;
	driveinfo *Drive;
	union REGS Regs;

	printk("ATAPI driver/demo code.\n");
/* make sure it's really DOS */
	Regs.x.ax=0x1600;
	int86(0x2F, &Regs, &Regs);
	if(Regs.h.al != 0 && Regs.h.al != 0x80)
	{	printk("Detected Windows version ");
		if(Regs.h.al == 0x01 || Regs.h.al == 0xFF)
			printk("2.x");
		else printk("%u.%u", Regs.h.al, Regs.h.ah);
		printk(", aborting\n");
		return(-1); }
/* install our interrupt handlers */
	SAVE_VECT(IRQ14_VECTOR, Vector14);
	SAVE_VECT(IRQ15_VECTOR, Vector15);
	SET_VECT(IRQ14_VECTOR, irq14);
	SET_VECT(IRQ15_VECTOR, irq15);
/* enable IRQ14 and IRQ15 at the 2nd 8259 PIC chip */
	outportb(0xA1, inportb(0xA1) & ~0xC0);
/* seed RNG */
	srand(time(NULL));
/* identify drives */
	ataProbe();
/* find an ATAPI CD-ROM drive */
	for(WhichDrive=0; WhichDrive < 4; WhichDrive++)
	{	Drive=_Drives + WhichDrive;
		if(Drive->IOAdr == 0)
			continue;
		printk("\n%s drive on I/F 0x%03X...\n", Drive->DrvSel ==
			0xA0 ? "master" : "slave", Drive->IOAdr);
		if(Drive->IsATAPI)
			break;
		printk("(ignoring ATA drive)\n"); }
/* if you find a drive, try playing with it */
	if(WhichDrive < 4)
	{	if(demoDataCD(Drive) != 0)
			demoAudioCD(Drive); }
	else printk("No ATAPI CD-ROM detected\n");
/* restore old interrupt handlers */
	RESTORE_VECT(IRQ14_VECTOR, Vector14);
	RESTORE_VECT(IRQ15_VECTOR, Vector15);
	return(0); }
#endif
