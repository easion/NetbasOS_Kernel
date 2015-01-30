
#include <drv/drv.h>
#include <drv/sym.h>
#include <drv/pci.h>
#include <drv/spin.h>
#include <drv/errno.h>

#define DMA_NUM_CHANNELS	0x08
#define DMA_1_MASK		0x0A
#define DMA_1_MODE		0x0B
#define DMA_1_FF		0x0C

#define DMA_2_MASK		0xD4
#define DMA_2_MODE		0xD6
#define DMA_2_FF		0xD8

#define dma_out( reg, val )	outb( ( reg ), ( val ) )
#define dma_in( reg )		inb( ( reg ) )

typedef struct {
    u32_t dc_nlock;
    unsigned int dc_naddress;
    unsigned int dc_ncount;
    unsigned int dc_npage;
} DMAChannel_s;

DMAChannel_s g_asDMAChannels[ DMA_NUM_CHANNELS ] = {
    /* 8 bit channels */
    { 0, 0x00, 0x01, 0x87 },
    { 0, 0x02, 0x03, 0x83 },
    { 0, 0x04, 0x05, 0x81 },
    { 0, 0x06, 0x07, 0x82 },
    /* 16 bit channels */
    { 1, 0xC0, 0xC2, 0x00 },	/* cascade */
    { 0, 0xC4, 0xC6, 0x8B },
    { 0, 0xC8, 0xCA, 0x89 },
    { 0, 0xCC, 0xCE, 0x8A }
};

int request_dma( unsigned int channel_nr ) {
    if ( channel_nr >= DMA_NUM_CHANNELS ) {
	return -1;
    }
    if ( atomic_swap( &g_asDMAChannels[ channel_nr ].dc_nlock, 1 ) != 0 ) {
	return -EBUSY;
    }
    return 0;
}

int free_dma( unsigned int channel_nr ) {
    if ( channel_nr >= DMA_NUM_CHANNELS ) {
	return -1;
    }
    if ( atomic_swap( &g_asDMAChannels[ channel_nr ].dc_nlock, 0 ) == 0 ) {
	return -1;
    }
    return 0;
}

int enable_dma_channel( unsigned int channel_nr ) {
    if ( channel_nr >= DMA_NUM_CHANNELS ) {
	return -1;
    }
    dma_out( ( channel_nr < 4 ) ? DMA_1_MASK : DMA_2_MASK, channel_nr & 3 );

    return 0;
}

int disable_dma_channel( unsigned int channel_nr ) {
    if ( channel_nr >= DMA_NUM_CHANNELS ) {
	return -1;
    }
    dma_out( ( channel_nr < 4 ) ? DMA_1_MASK : DMA_2_MASK, ( channel_nr & 3 )
     | 4 );

    return 0;
}

int clear_dma_ff( unsigned int channel_nr ) {
    if ( channel_nr >= DMA_NUM_CHANNELS ) {
	return -1;
    }
    dma_out( ( channel_nr < 4 ) ? DMA_1_FF : DMA_2_FF, 0 );

    return 0;
}

int set_dma_mode( unsigned int channel_nr, u8_t nMode ) {
    if ( channel_nr >= DMA_NUM_CHANNELS ) {
	return -1;
    }
    dma_out( ( channel_nr < 4 ) ? DMA_1_MODE : DMA_2_MODE, ( channel_nr & 3 )
	| nMode );

    return 0;
}

int set_dma_page( unsigned int channel_nr, u8_t nPage ) {
    if ( channel_nr >= DMA_NUM_CHANNELS ) {
	return -1;
    }
    dma_out( g_asDMAChannels[ channel_nr ].dc_npage, ( channel_nr < 4 ) ?
	nPage : nPage & 0xFE );

    return 0;
}

int set_dma_address( unsigned int channel_nr, void *pAddress ) {
    register unsigned int nAddr = ( unsigned int ) pAddress;

    if ( channel_nr >= DMA_NUM_CHANNELS ) {
	return -1;
    }
    if ( channel_nr < 4 ) {
	dma_out( g_asDMAChannels[ channel_nr ].dc_naddress, nAddr & 0xFF );
	dma_out( g_asDMAChannels[ channel_nr ].dc_naddress, ( nAddr >> 8 )
	    & 0xFF );
    } else {
	dma_out( g_asDMAChannels[ channel_nr ].dc_naddress, ( nAddr >> 1 )
	    & 0xFF );
	dma_out( g_asDMAChannels[ channel_nr ].dc_naddress, ( nAddr >> 9 )
	    & 0xFF );
    }

    return 0;
}

int set_dma_count( unsigned int channel_nr, u16_t nCount ) {
    register u16_t i = nCount - 1;

    if ( channel_nr >= DMA_NUM_CHANNELS ) {
	return -1;
    }
    if ( channel_nr < 4 ) {
	dma_out( g_asDMAChannels[ channel_nr ].dc_ncount, i & 0xFF );
	dma_out( g_asDMAChannels[ channel_nr ].dc_ncount, ( i >> 8 ) & 0xFF );
    } else {
	dma_out( g_asDMAChannels[ channel_nr ].dc_ncount, ( i >> 1 ) & 0xFF );
	dma_out( g_asDMAChannels[ channel_nr ].dc_ncount, ( i >> 9 ) & 0xFF );
    }

    return 0;
}
