
/*
 *  SoundBlaster Pro Driver for JICAMA OS
 *  Copyright (C) 2005  Easion
 
Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions 
are met: 
1. Redistributions of source code must retain the above copyright 
   notice, this list of conditions and the following disclaimer. 
2. Redistributions in binary form must reproduce the above copyright 
   notice, this list of conditions and the following disclaimer in the 
   documentation and/or other materials provided with the distribution. 
3. Neither the name of Axon Digital Design nor the names of its contributors 
   may be used to endorse or promote products derived from this software 
   without specific prior written permission. 

THIS SOFTWARE IS PROVIDED BY AXON DIGITAL DESIGN AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE 
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS 
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY 
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
SUCH DAMAGE. 
*/
/*
 *  SoundBlaster Pro Driver for the AtheOS kernel
 *  Copyright (C) 2000  Joel Smith
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Revision History :
 *		19/08/00      Initial version           Joel Smith
 *							<joels@@mobyfoo.org>
 *
 *		29/12/00      Fixed for AtheOS 0.3.0    Kristian Van Der Vliet
 *		08/07/02      Initalise correctly	Kristian Van Der Vliet
 */
                 
#include <drv/drv.h>
#include <drv/sym.h>
#include <drv/pci.h>
#include <drv/spin.h>
#include <drv/errno.h>

#define dsp_out( reg, val )	outb( reg, val )
#define dsp_in( reg )		inb( reg )

#define SB_TIMEOUT		64000
#define SB_DMA_SIZE		( 1024 )
#define SB_IRQ			5
#define SB_DMA			1
#define SB_DMA_PLAY		0x58
#define SB_DMA_REC		0x54

#define SB_MIN_SPEED		4000
#define SB_MAX_SPEED		44100
#define SB_DEFAULT_SPEED	22050

#define SB_PORT_BASE		0x220

#define SB_MIXER_ADDR		( SB_PORT_BASE + 0x04 )
#define SB_MIXER_DATA		( SB_PORT_BASE + 0x05 )

#define SB_DSP_RESET		( SB_PORT_BASE + 0x06 )
#define SB_DSP_READ		( SB_PORT_BASE + 0x0A )
#define SB_DSP_COMMAND		( SB_PORT_BASE + 0x0C )
#define SB_DSP_STATUS		( SB_PORT_BASE + 0x0C )
#define SB_DSP_DATA_AVL		( SB_PORT_BASE + 0x0E )
#define SB_DSP_DATA_AVL16	( SB_PORT_BASE + 0x0F )

#define SB_DSP_CMD_TCONST	0x40
#define SB_DSP_CMD_HSSIZE	0x48
#define SB_DSP_CMD_HSDAC	0x91
#define SB_DSP_CMD_HSADC	0x99
#define SB_DSP_CMD_DAC8		0x14
#define SB_DSP_CMD_ADC8		0x24
#define SB_DSP_CMD_SPKON	0xD1
#define SB_DSP_CMD_SPKOFF	0xD3
#define SB_DSP_CMD_GETVER	0xE1

#define DEV_READ		0
#define DEV_WRITE		1

static int id = -1;
static void* isr_handler;

int dsp_stereo;
int dsp_speed;
int dsp_in_use = 0;
int dsp_highspeed = 0;
u8_t dsp_tconst = 0;
char *dma_buffer;
int open_mode = 0;
int version[ 2 ];
int irq_lock = 0;
int sb_bits = 8;


void enable_dma(unsigned int dmanr);
void disable_dma(unsigned int dmanr);
void isadma_stopdma (int channelnr);
int isadma_startdma (int channelnr, void *addr, size_t len, int mode);
 void set_dma_addr(unsigned int dmanr, unsigned int a);
void set_dma_page(unsigned int dmanr, char pagenr);
 void set_dma_mode(unsigned int dmanr, char mode);
 void set_dma_count(unsigned int dmanr, unsigned int count);
 void clear_dma_ff(unsigned int dmanr);
int deinit_SoundBlaster( void ) ;


CREATE_SPINLOCK( sb_lock );

#define SOUND_PCM_WRITE_CHANNELS	0
#define SOUND_PCM_WRITE_RATE		1
#define SOUND_PCM_WRITE_BITS		2
#define SNDCTL_DSP_SYNC			3

#define SNDCTL_DSP_SETFMT		4

#define SOUND_MIXER_READ_DEVMASK	5
#define SOUND_MIXER_WRITE_VOLUME	6
#define SOUND_MASK_PCM			7
#define SOUND_MIXER_WRITE_PCM		8
#define SNDCTL_DSP_STEREO		9
#define SNDCTL_DSP_SPEED		10
#define SNDCTL_DSP_GETBLKSIZE		11
#define SNDCTL_DSP_RESET		12
#define SOUND_PCM_READ_RATE		13
#define SOUND_PCM_READ_CHANNELS		14
#define SNDCTL_DSP_GETFMTS		15

#define AFMT_S16_LE			0x00000010

int init_SoundBlaster( void ) ;
int sb_write( dev_prvi_t* devfp, off_t  pos, char *buffer, int size ) ;
int sb_read( dev_prvi_t* devfp, off_t  pos,  char *buffer, int size ) ;
int sb_open( char *f, int mode);
int sb_ioctl( dev_prvi_t* devfp, unsigned int cmd, unsigned int arg,int );
int sb_close( dev_prvi_t* devfp ) ;

#define DSP_MIXER_ADDRESS_PORT	0x04
#define DSP_MIXER_DATA_PORT		0x05
#define DSP_RESET_PORT 				0x06
#define DSP_READ_DATA_PORT 		0x0A
#define DSP_WRITE_PORT 				0x0C		//Same port used for reading status and writing data
#define DSP_READ_STATUS_PORT 	0x0E

unsigned char reset_dsp(unsigned short base_address)
{
	int delay;

	outb(base_address+DSP_RESET_PORT,1);
	for(delay=0;delay<0xffff;delay++);

	outb(base_address+DSP_RESET_PORT,0);
	for(delay=0;delay<0xffff;delay++);

	if((inb(base_address+DSP_READ_STATUS_PORT)&0x80)==0) return 1;

	if(inb(base_address+DSP_READ_DATA_PORT)!=0xAA) return 1;

	return 0;
}

#ifdef __DLL__
unsigned short  dsp_base;

int dll_main(char **argv)
{
unsigned short base;
	static const driver_ops_t ops =
	{
		d_name:		"SB16",
		d_author:	"Joel Smith",
		d_kver:	CURRENT_KERNEL_VERSION,
		d_version:	"0.02",
		d_index:	0x9200,
		open:		sb_open,
		close:		sb_close,
		read:		sb_read,
		write:		sb_write,
		ioctl:		sb_ioctl,			

	};

	for(base=0x200;base<0x280;base+=0x10)	//Tries to reset all DSP addresses there is
		if(reset_dsp(base)==0)
		{
			dsp_base=base;
			printk("Find DSP port:%x \n",dsp_base);
			break;
		}

		if (base==0x280)	{return -1;}

	init_SoundBlaster();

	if(kernel_driver_register(&ops)!=OK){
		printk("register sb16 failed");
		return -1;
	}

	return 0;
}


int dll_destroy()
{
	return deinit_SoundBlaster();
}

int dll_version()
{
	printk("SoundBlaster Pro Driver\n");
	return 0;
}
#endif

void sb_mixer_write( int reg, int val )
{
    dsp_out( SB_MIXER_ADDR, reg );
    dsp_out( SB_MIXER_DATA, val );
}

void sb_volume_dsp( int left, int right )
{
    sb_mixer_write( 0x02, ( left << 4 ) + right );
}

void sb_volume_master( int left, int right )
{
    sb_mixer_write( 0x22, ( left << 4 ) + right );
}

void sb_volume_voice( int left, int right )
{
    sb_mixer_write( 0x04, ( left << 4 ) + right );
}

void sb_volume_fm( int left, int right )
{
    sb_mixer_write( 0x26, ( left << 4 ) + right );
}

void sb_volume_cd( int left, int right )
{
    sb_mixer_write( 0x28, ( left << 4 ) + right );
}

void sb_volume_linein( int left, int right )
{
    sb_mixer_write( 0x2E, ( left << 4 ) + right );
}

int sb_dsp_command( int com ) {
    register int i;

    for ( i = 0; i < SB_TIMEOUT; i++ ) {
	if ( ( dsp_in( SB_DSP_STATUS ) & 0x80 ) == 0 ) {
	    dsp_out( SB_DSP_COMMAND, com );
	    return 0;
	}
    }

    printk( "DSP command timeout.\n" );
    return -1;
}

int sb_dsp_reset( void ) {
    int i;

    dsp_out( SB_DSP_RESET, 1 );
    for ( i = 0; i < SB_TIMEOUT; i++ );

    dsp_out( SB_DSP_RESET, 0 );
    for ( i = 0; i < SB_TIMEOUT; i++ );

    for ( i = 0; i < SB_TIMEOUT && !( dsp_in( SB_DSP_DATA_AVL ) & 0x80 ); i++ );

    if ( dsp_in( SB_DSP_READ ) != 0xAA ) {
	printk( "Reset failed.\n" );
	return -1;
    }

    printk( "Reset OK.\n" );

    return 0;
}

void sb_dsp_speaker( int state ) {
    if ( state ) {
	sb_dsp_command( SB_DSP_CMD_SPKON );
    } else {
	sb_dsp_command( SB_DSP_CMD_SPKOFF );
    }
}

int sb_dsp_init( void ) {
    int i;

    if ( sb_dsp_reset() ) {
	printk( "SoundBlaster card not found.\n" );
	return -1;
    }

    version[ 0 ] = version[ 1 ] = 0;
    sb_dsp_command( SB_DSP_CMD_GETVER );

    for ( i = 0; i < SB_TIMEOUT; i++ ) {
	if ( dsp_in( SB_DSP_DATA_AVL ) & 0x80 ) {
	    if ( version[ 0 ] == 0 ) {
		version[ 0 ] = dsp_in( SB_DSP_READ );
	    } else {
		version[ 1 ] = dsp_in( SB_DSP_READ );
		break;
	    }
	}
    }

    printk( "DSP version %d.%d\n", version[ 0 ], version[ 1 ] );

    return 0;
}

int sb_irq( )
{
    if ( sb_bits == 16 ) {
	dsp_in( SB_DSP_DATA_AVL16 );
    } else {
	dsp_in( SB_DSP_DATA_AVL );
    }
//#waring
    free_dma( SB_DMA );  /*reset*/
    sb_dsp_speaker( 0 );

    irq_lock = 0;
    outb( 0x20, 0x20 );
    outb( 0xa0, 0x20 );
	return 0;
}



int sb_set_irq( void ) {

    isr_handler = put_irq_handler(SB_IRQ, ( u32_t ) &sb_irq ,NULL,"dsp");

    return 0;
}

int sb_dsp_set_stereo( int stereo ) {
    if ( version[ 0 ] < 3 ) {
    printk( "error: version<3 [%s]: stereo %x\n", __FUNCTION__, stereo );
	return 0;
    }
    dsp_stereo = ( stereo <= 0 ) ? 0 : 1;
    printk( "[%s]: dsp_stereo %x\n", __FUNCTION__, dsp_stereo );

    return dsp_stereo;
}

int sb_dsp_speed( int speed ) {
    int error = 0;
    u8_t tconst;

    if ( speed < SB_MIN_SPEED ) {
	speed = SB_MIN_SPEED;
	error = 1;
    } else if ( speed > SB_MAX_SPEED ) {
	speed = SB_MAX_SPEED;
	error = 1;
    }

    if ( dsp_stereo && speed > 22050 ) {
	speed = 22050;
	error = 1;
    }

    if ( dsp_stereo ) {
	speed *= 2;
    }

    if ( speed > 22050 ) {
	tconst = ( u8_t ) ( ( 65536 - ( ( 256000000 + speed / 2 ) /
	    speed ) ) >> 8 );
	dsp_highspeed = 1;

	dsp_tconst = tconst;

	speed = 65536 - ( tconst << 8 );
	speed = ( 256000000 + speed / 2 ) / speed;
    } else {
	tconst = ( 256 - ( ( 1000000 + speed / 2 ) / speed ) ) & 0xFF;
	dsp_highspeed = 0;

	dsp_tconst = tconst;

	speed = 256 - tconst;
	speed = ( 1000000 + speed / 2 ) / speed;
    }

    if ( dsp_stereo ) {
	speed /= 2;
    }

    dsp_speed = speed;

	if (error!=0)
	{
	    printk( "[%s]: error: %d speed %d\n", __FUNCTION__, error, speed );
	}

    return ( error != 0 ) ? -EINVAL : speed;
}

void disable_dma(unsigned int dmanr);

int sb_dma_setup( void *address, int count ) {
    /* should lock here. */
    spin_lock( &sb_lock );

	if ( request_dma( SB_DMA ) < 0 ) {
	printk( "Request of DMA channel Failed.\n" );
	return -EBUSY;
    }   

    disable_dma_channel( SB_DMA );
    clear_dma_ff( SB_DMA );

    set_dma_mode( SB_DMA, ( open_mode == DEV_WRITE ) ? SB_DMA_PLAY :
	SB_DMA_REC );
    set_dma_address( SB_DMA, address );
    set_dma_page( SB_DMA, ( int ) address >> 16 );
    set_dma_count( SB_DMA, count );

    enable_dma_channel( SB_DMA );

    spin_unlock( &sb_lock );

    return 0;
}

int sb_dsp_setup( int count ) {
    count--;
    spin_lock( &sb_lock );

    if ( open_mode == DEV_WRITE ) {
	if ( dsp_highspeed ) {
	    sb_dsp_command( SB_DSP_CMD_HSSIZE );
	    sb_dsp_command( ( u8_t ) ( count & 0xFF ) );
	    sb_dsp_command( ( u8_t ) ( ( count >> 8 ) & 0xFF ) );
	    sb_dsp_command( SB_DSP_CMD_HSDAC );
	} else {
	    sb_dsp_command( SB_DSP_CMD_DAC8 );
	    sb_dsp_command( ( u8_t ) ( count & 0xFF ) );
	    sb_dsp_command( ( u8_t ) ( ( count >> 8 ) & 0xFF ) );
	}
    } else {
	if ( dsp_highspeed ) {
	    sb_dsp_command( SB_DSP_CMD_HSSIZE );
	    sb_dsp_command( ( u8_t ) ( count & 0xFF ) );
	    sb_dsp_command( ( u8_t ) ( ( count >> 8 ) & 0xFF ) );
	    sb_dsp_command( SB_DSP_CMD_HSADC );
	} else {
	    sb_dsp_command( SB_DSP_CMD_ADC8 );
	    sb_dsp_command( ( u8_t ) ( count & 0xFF ) );
	    sb_dsp_command( ( u8_t ) ( ( count >> 8 ) & 0xFF ) );
	}
    }

    spin_unlock( &sb_lock );

    return 0;
}

int sb_write( dev_prvi_t* devfp, off_t  pos, char *buffer, int size ) 
{
    int count;
    u8_t i;

    //printk( "dsp_stereo: %d\n", dsp_stereo );
    if ( size > SB_DMA_SIZE ) {
	count = SB_DMA_SIZE;
    } else {
	count = size;
    }

	printk("write size: %d\n", count);

    open_mode = DEV_WRITE;
	while( atomic_swap( (int *)&irq_lock, 1 ) != 0 )

//#warning FIXME!
    //enable();
    /* Wait for last transfer to complete. */
   //while ( atomic_swap( ( u32_t * ) &irq_lock, 1 ) != 0 );

    memcpy( dma_buffer, buffer, count );

    if ( dsp_stereo ) {
	dsp_out( SB_MIXER_ADDR, 0x0E );
	i = dsp_in( SB_MIXER_DATA );
	dsp_out( SB_MIXER_ADDR, 0x0E );
	dsp_out( SB_MIXER_DATA, i | 0x02 );
    }

    sb_dsp_command( SB_DSP_CMD_TCONST );
    sb_dsp_command( dsp_tconst );
    sb_dsp_speaker( 1 );

    sb_dma_setup( dma_buffer, count );
    sb_dsp_setup( count );

    return count;
}

int sb_read( dev_prvi_t* devfp, off_t  pos,  char *buffer, int size )
{
    int count;

	while( atomic_swap( (int *)&irq_lock, 1 ) != 0 )

    if ( size > SB_DMA_SIZE ) {
	count = SB_DMA_SIZE;
    } else {
	count = size;
    }

    open_mode = DEV_READ;

    if ( dsp_stereo ) {
	sb_dsp_command( 0xA8 );
    } else {
	sb_dsp_command( 0xA0 );
    }
    sb_dsp_command( SB_DSP_CMD_TCONST );
    sb_dsp_command( dsp_tconst );

    sb_dma_setup( dma_buffer, count );
    sb_dsp_setup( count );

    /* UUUHHH, Wait until DMA is complete. */
      while( atomic_swap( (int *)&irq_lock, 1 ) != 0 )
                        /* do nothing. */ ;
        
    memcpy( ( char * ) buffer, dma_buffer, count );

    return count;
}

int sb_ioctl( dev_prvi_t* devfp, unsigned int cmd, unsigned int arg, int bfromkernel )
{
    printk( "[%s]: cmd %d, arg%d\n", __FUNCTION__, cmd, arg );
    switch ( cmd ) {
	case SNDCTL_DSP_SPEED:
	case SOUND_PCM_WRITE_RATE:
	    return sb_dsp_speed( arg );
	case SOUND_PCM_READ_RATE:
	    return dsp_speed;
	case SOUND_PCM_WRITE_CHANNELS:
	    return sb_dsp_set_stereo( arg );
	case SOUND_PCM_READ_CHANNELS:
	    return dsp_stereo + 1;
	case SNDCTL_DSP_STEREO:
	    return sb_dsp_set_stereo( arg );
	case SNDCTL_DSP_GETBLKSIZE:
	    return SB_DMA_SIZE;
	case SNDCTL_DSP_RESET:
	    return sb_dsp_reset();
	case SNDCTL_DSP_GETFMTS: /* RECHECK HERE */
	    return sb_bits;
	case SNDCTL_DSP_SETFMT:
	    if ( arg != 8 && arg != 16 ) {
		return -EINVAL;
	    }
	    sb_bits = arg;
	    return 1;
	case SOUND_PCM_WRITE_BITS:
	    if ( arg != 8 && arg != 16 ) {
		return -EINVAL;
	    }
	    sb_bits = arg;
	    return 1;
	default:
	    return -EINVAL;
    }

    return -EINVAL;
}

int sb_open( char *f, int mode)
{
    if ( dsp_in_use == 1 ) {
	printk( "Device already in use.\n" );
	return -EBUSY;
    }

    if ( sb_dsp_reset() ) {
	return -EIO;
    }

    dsp_stereo = 0;
    dsp_speed = SB_DEFAULT_SPEED;
    sb_dsp_speed( dsp_speed );
    irq_lock = 0;

    dsp_in_use = 1;

    return 0;
}

int sb_close( dev_prvi_t* devfp ) 
{
    //printk( "Closing device.\n" );

   free_dma( SB_DMA );
    dsp_in_use = 0;

    return 0;
}

int init_SoundBlaster( void ) 
{
    if ( sb_dsp_init() ) {
	return -1;
    }

    if ( ( dma_buffer = low_alloc( SB_DMA_SIZE ) ) == NULL ) {
		printk( "Out of memory.\n" );
		return -ENOMEM;
    }

    sb_set_irq();

    sb_volume_dsp( 15, 15 );
    sb_volume_master( 15, 15 );
    sb_volume_voice( 15, 15 );
    sb_volume_fm( 15, 15 );
    sb_volume_cd( 15, 15 );
    sb_volume_linein( 15, 15 );

    return 0;
}

int deinit_SoundBlaster( void ) 
{
	free_irq_handler(SB_IRQ,isr_handler);
    return 0;
}


