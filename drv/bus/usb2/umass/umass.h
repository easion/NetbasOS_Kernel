
//#include <sisc.h>
#include <scsi_common.h>
#include "cam.h"

#if 1
/* this enables loading of virtual buffers into DMA */
#define	UMASS_USB_FLAGS .ext_buffer=1,
#else
#define	UMASS_USB_FLAGS
#endif

#if USB_DEBUG
#define	DIF(m, x)				\
  do {						\
    if (umass_debug & (m)) { x ; }		\
  } while (0)

#define	DPRINTF(sc, m, fmt, ...)			\
  do {							\
    if (umass_debug & (m)) {				\
        printf("%s:%s: " fmt,				\
	       (sc) ? (const char *)(sc)->sc_name :	\
	       (const char *)"umassX",			\
		__FUNCTION__ ,## __VA_ARGS__);		\
    }							\
  } while (0)

#define	UDMASS_GEN	0x00010000	/* general */
#define	UDMASS_SCSI	0x00020000	/* scsi */
#define	UDMASS_UFI	0x00040000	/* ufi command set */
#define	UDMASS_ATAPI	0x00080000	/* 8070i command set */
#define	UDMASS_CMD	(UDMASS_SCSI|UDMASS_UFI|UDMASS_ATAPI)
#define	UDMASS_USB	0x00100000	/* USB general */
#define	UDMASS_BBB	0x00200000	/* Bulk-Only transfers */
#define	UDMASS_CBI	0x00400000	/* CBI transfers */
#define	UDMASS_WIRE	(UDMASS_BBB|UDMASS_CBI)
#define	UDMASS_ALL	0xffff0000	/* all of the above */
static int umass_debug = 0;

SYSCTL_NODE(_hw_usb2, OID_AUTO, umass, CTLFLAG_RW, 0, "USB umass");
SYSCTL_INT(_hw_usb2_umass, OID_AUTO, debug, CTLFLAG_RW,
    &umass_debug, 0, "umass debug level");
#else
#define	DIF(...) do { } while (0)
#define	DPRINTF(...) do { } while (0)
#endif

#define	UMASS_GONE ((struct umass_softc *)1)
#define	UMASS_MAXUNIT 64		/* XXX temporary */

#define	UMASS_BULK_SIZE (1 << 17)
#define	UMASS_CBI_DIAGNOSTIC_CMDLEN 12	/* bytes */
#define	UMASS_MAX_CMDLEN MAX(12, CAM_MAX_CDBLEN)	/* bytes */

/* USB transfer definitions */

#define	UMASS_T_BBB_RESET1      0	/* Bulk-Only */
#define	UMASS_T_BBB_RESET2      1
#define	UMASS_T_BBB_RESET3      2
#define	UMASS_T_BBB_COMMAND     3
#define	UMASS_T_BBB_DATA_READ   4
#define	UMASS_T_BBB_DATA_RD_CS  5
#define	UMASS_T_BBB_DATA_WRITE  6
#define	UMASS_T_BBB_DATA_WR_CS  7
#define	UMASS_T_BBB_STATUS      8
#define	UMASS_T_BBB_MAX         9

#define	UMASS_T_CBI_RESET1      0	/* CBI */
#define	UMASS_T_CBI_RESET2      1
#define	UMASS_T_CBI_RESET3      2
#define	UMASS_T_CBI_COMMAND     3
#define	UMASS_T_CBI_DATA_READ   4
#define	UMASS_T_CBI_DATA_RD_CS  5
#define	UMASS_T_CBI_DATA_WRITE  6
#define	UMASS_T_CBI_DATA_WR_CS  7
#define	UMASS_T_CBI_STATUS      8
#define	UMASS_T_CBI_RESET4      9
#define	UMASS_T_CBI_MAX        10

#define	UMASS_T_MAX MAX(UMASS_T_CBI_MAX, UMASS_T_BBB_MAX)

/* Generic definitions */

/* Direction for transfer */
#define	DIR_NONE	0
#define	DIR_IN		1
#define	DIR_OUT		2

/* device name */
#define	DEVNAME		"umass"
#define	DEVNAME_SIM	"umass-sim"

/* Approximate maximum transfer speeds (assumes 33% overhead). */
#define	UMASS_FULL_TRANSFER_SPEED	1000
#define	UMASS_HIGH_TRANSFER_SPEED	40000
#define	UMASS_FLOPPY_TRANSFER_SPEED	20

#define	UMASS_TIMEOUT			5000	/* ms */

/* CAM specific definitions */

#define	UMASS_SCSIID_MAX	1	/* maximum number of drives expected */
#define	UMASS_SCSIID_HOST	UMASS_SCSIID_MAX

/* Bulk-Only features */

#define	UR_BBB_RESET		0xff	/* Bulk-Only reset */
#define	UR_BBB_GET_MAX_LUN	0xfe	/* Get maximum lun */

/* Command Block Wrapper */
typedef struct {
	uDWord	dCBWSignature;
#define	CBWSIGNATURE	0x43425355
	uDWord	dCBWTag;
	uDWord	dCBWDataTransferLength;
	uByte	bCBWFlags;
#define	CBWFLAGS_OUT	0x00
#define	CBWFLAGS_IN	0x80
	uByte	bCBWLUN;
	uByte	bCDBLength;
#define	CBWCDBLENGTH	16
	uByte	CBWCDB[CBWCDBLENGTH];
} __packed umass_bbb_cbw_t;

#define	UMASS_BBB_CBW_SIZE	31

/* Command Status Wrapper */
typedef struct {
	uDWord	dCSWSignature;
#define	CSWSIGNATURE	0x53425355
#define	CSWSIGNATURE_IMAGINATION_DBX1	0x43425355
#define	CSWSIGNATURE_OLYMPUS_C1	0x55425355
	uDWord	dCSWTag;
	uDWord	dCSWDataResidue;
	uByte	bCSWStatus;
#define	CSWSTATUS_GOOD	0x0
#define	CSWSTATUS_FAILED	0x1
#define	CSWSTATUS_PHASE	0x2
} __packed umass_bbb_csw_t;

#define	UMASS_BBB_CSW_SIZE	13

/* CBI features */

#define	UR_CBI_ADSC	0x00

typedef union {
	struct {
		uint8_t	type;
#define	IDB_TYPE_CCI		0x00
		uint8_t	value;
#define	IDB_VALUE_PASS		0x00
#define	IDB_VALUE_FAIL		0x01
#define	IDB_VALUE_PHASE		0x02
#define	IDB_VALUE_PERSISTENT	0x03
#define	IDB_VALUE_STATUS_MASK	0x03
	} __packed common;

	struct {
		uint8_t	asc;
		uint8_t	ascq;
	} __packed ufi;
} __packed umass_cbi_sbl_t;

struct umass_softc;			/* see below */

typedef void (umass_callback_t)(struct umass_softc *sc, union ccb *ccb,
    	uint32_t residue, uint8_t status);

#define	STATUS_CMD_OK		0	/* everything ok */
#define	STATUS_CMD_UNKNOWN	1	/* will have to fetch sense */
#define	STATUS_CMD_FAILED	2	/* transfer was ok, command failed */
#define	STATUS_WIRE_FAILED	3	/* couldn't even get command across */

typedef uint8_t (umass_transform_t)(struct umass_softc *sc, uint8_t *cmd_ptr,
    	uint8_t cmd_len);

struct umass_devdescr {
	uint32_t vid;
#define	VID_WILDCARD	0xffffffff
#define	VID_EOT		0xfffffffe
	uint32_t pid;
#define	PID_WILDCARD	0xffffffff
#define	PID_EOT		0xfffffffe
	uint32_t rid;
#define	RID_WILDCARD	0xffffffff
#define	RID_EOT		0xfffffffe

	/* wire and command protocol */
	uint16_t proto;
#define	UMASS_PROTO_BBB		0x0001	/* USB wire protocol */
#define	UMASS_PROTO_CBI		0x0002
#define	UMASS_PROTO_CBI_I	0x0004
#define	UMASS_PROTO_WIRE		0x00ff	/* USB wire protocol mask */
#define	UMASS_PROTO_SCSI		0x0100	/* command protocol */
#define	UMASS_PROTO_ATAPI	0x0200
#define	UMASS_PROTO_UFI		0x0400
#define	UMASS_PROTO_RBC		0x0800
#define	UMASS_PROTO_COMMAND	0xff00	/* command protocol mask */

	/* Device specific quirks */
	uint16_t quirks;
#define	NO_QUIRKS		0x0000
	/*
	 * The drive does not support Test Unit Ready. Convert to Start Unit
	 */
#define	NO_TEST_UNIT_READY	0x0001
	/*
	 * The drive does not reset the Unit Attention state after REQUEST
	 * SENSE has been sent. The INQUIRY command does not reset the UA
	 * either, and so CAM runs in circles trying to retrieve the initial
	 * INQUIRY data.
	 */
#define	RS_NO_CLEAR_UA		0x0002
	/* The drive does not support START STOP.  */
#define	NO_START_STOP		0x0004
	/* Don't ask for full inquiry data (255b).  */
#define	FORCE_SHORT_INQUIRY	0x0008
	/* Needs to be initialised the Shuttle way */
#define	SHUTTLE_INIT		0x0010
	/* Drive needs to be switched to alternate iface 1 */
#define	ALT_IFACE_1		0x0020
	/* Drive does not do 1Mb/s, but just floppy speeds (20kb/s) */
#define	FLOPPY_SPEED		0x0040
	/* The device can't count and gets the residue of transfers wrong */
#define	IGNORE_RESIDUE		0x0080
	/* No GetMaxLun call */
#define	NO_GETMAXLUN		0x0100
	/* The device uses a weird CSWSIGNATURE. */
#define	WRONG_CSWSIG		0x0200
	/* Device cannot handle INQUIRY so fake a generic response */
#define	NO_INQUIRY		0x0400
	/* Device cannot handle INQUIRY EVPD, return CHECK CONDITION */
#define	NO_INQUIRY_EVPD		0x0800
	/* Pad all RBC requests to 12 bytes. */
#define	RBC_PAD_TO_12		0x1000
	/*
	 * Device reports number of sectors from READ_CAPACITY, not max
	 * sector number.
	 */
#define	READ_CAPACITY_OFFBY1	0x2000
	/*
	 * Device cannot handle a SCSI synchronize cache command.  Normally
	 * this quirk would be handled in the cam layer, but for IDE bridges
	 * we need to associate the quirk with the bridge and not the
	 * underlying disk device.  This is handled by faking a success
	 * result.
	 */
#define	NO_SYNCHRONIZE_CACHE	0x4000
};




struct umass_softc {

	struct scsi_sense cam_scsi_sense;
	struct scsi_test_unit_ready cam_scsi_test_unit_ready;
	struct mtx sc_mtx;
	struct {
		uint8_t *data_ptr;
		union ccb *ccb;
		umass_callback_t *callback;

		uint32_t data_len;	/* bytes */
		uint32_t data_rem;	/* bytes */
		uint32_t data_timeout;	/* ms */
		uint32_t actlen;	/* bytes */

		uint8_t	cmd_data[UMASS_MAX_CMDLEN];
		uint8_t	cmd_len;	/* bytes */
		uint8_t	dir;
		uint8_t	lun;
	}	sc_transfer;

	/* Bulk specific variables for transfers in progress */
	umass_bbb_cbw_t cbw;		/* command block wrapper */
	umass_bbb_csw_t csw;		/* command status wrapper */

	/* CBI specific variables for transfers in progress */
	umass_cbi_sbl_t sbl;		/* status block */

	device_t sc_dev;
	struct usb2_device *sc_udev;
	struct cam_sim *sc_sim;		/* SCSI Interface Module */
	struct usbd_xfer *sc_xfer[UMASS_T_MAX];

	/*
	 * The command transform function is used to convert the SCSI
	 * commands into their derivatives, like UFI, ATAPI, and friends.
	 */
	umass_transform_t *sc_transform;

	uint32_t sc_unit;

	uint16_t sc_proto;		/* wire and cmd protocol */
	uint16_t sc_quirks;		/* they got it almost right */

	uint8_t	sc_name[16];
	uint8_t	sc_iface_no;		/* interface number */
	uint8_t	sc_maxlun;		/* maximum LUN number, inclusive */
	uint8_t	sc_last_xfer_index;
	uint8_t	sc_status_try;
};

struct umass_probe_proto {
	uint16_t quirks;
	uint16_t proto;

	int32_t	error;
};
