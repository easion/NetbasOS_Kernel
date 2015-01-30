/* 
** Copyright 2002, Thomas Kurschel. All rights reserved.
** Distributed under the terms of the NewOS License.
*/

#ifndef __SCSI_CMDS_H__
#define __SCSI_CMDS_H__

#include <kernel/bus/scsi/lendian_bitfield.h>

/* always keep in mind that SCSI is big-endian !!! */

#define SCSI_STD_TIMEOUT 10

// SCSI status (as the result of a command)
#define SCSI_STATUS_GOOD (0 << 1)
#define SCSI_STATUS_CHECK_CONDITION (1 << 1)	// error occured
#define SCSI_STATUS_CONDITION_MET (2 << 1)		// "found" for SEARCH DATA and PREFETCH
#define SCSI_STATUS_BUSY (4 << 1)				// try again later (??? == QUEUE_FULL ???)
#define SCSI_STATUS_INTERMEDIATE (8 << 1)		// used by linked command only
#define SCSI_STATUS_INTERMEDIATE_COND_MET (10 << 1) // ditto
#define SCSI_STATUS_RESERVATION_CONFLICT (12 << 1) // only if RESERVE/RELEASE is used
#define SCSI_STATUS_COMMAND_TERMINATED (17 << 1) // aboted by TERMINATE I/O PROCESS
#define SCSI_STATUS_QUEUE_FULL (20 << 1)		// queue full

#define SCSI_STATUS_MASK 0xfe

// SCSI sense key
#define SCSIS_KEY_NO_SENSE 0
#define SCSIS_KEY_RECOVERED_ERROR 1
#define SCSIS_KEY_NOT_READY 2				// operator intervention may be required
#define SCSIS_KEY_MEDIUM_ERROR 3			// can be set if source could be hardware error
#define SCSIS_KEY_HARDWARE_ERROR 4
#define SCSIS_KEY_ILLEGAL_REQUEST 5			// invalid command
#define SCSIS_KEY_UNIT_ATTENTION 6			// medium changed or target reset
#define SCSIS_KEY_DATA_PROTECT 7			// data access forbidden
#define SCSIS_KEY_BLANK_CHECK 8				// tried to read blank or to write non-blank medium
#define SCSIS_KEY_VENDOR_SPECIFIC 9
#define SCSIS_KEY_COPY_ABORTED 10			// error in COPY or COMPARE command
#define SCSIS_KEY_ABORTED_COMMAND 11		// aborted by target, retry *may* help
#define SCSIS_KEY_EQUAL 12					// during SEARCH: data found
#define SCSIS_KEY_VOLUME_OVERFLOW 13		// tried to write buffered data beyond end of medium
#define SCSIS_KEY_MISCOMPARE 14
#define SCSIS_KEY_RESERVED 15

// SCSI ASC and ASCQ data - (ASC << 8) | ASCQ
// all codes with bit 7 of ASC or ASCQ set are vendor-specific
#define SCSIS_ASC_NO_SENSE 0x0000
#define SCSIS_ASC_IO_PROC_TERMINATED 0x0006
#define SCSIS_ASC_AUDIO_PLAYING 0x0011
#define SCSIS_ASC_AUDIO_PAUSED 0x0012
#define SCSIS_ASC_AUDIO_COMPLETED 0x0013
#define SCSIS_ASC_AUDIO_ERROR 0x0014		// playing has stopped due to error
#define SCSIS_ASC_AUDIO_NO_STATUS 0x0015
#define SCSIS_ASC_NO_INDEX 0x0100			// no index/sector signal
#define SCSIS_ASC_NO_SEEK_CMP 0x0200		// ???
#define SCSIS_ASC_WRITE_FAULT 0x0300
#define SCSIS_ASC_LUN_NOT_READY 0x0400		// LUN not ready, cause not reportable
#define SCSIS_ASC_LUN_BECOMING_READY 0x0401 // LUN in progress of becoming ready
#define SCSIS_ASC_LUN_NEED_INIT 0x0402		// LUN need initializing command
#define SCSIS_ASC_LUN_NEED_MANUAL_HELP 0x0403 // LUN needs manual intervention
#define SCSIS_ASC_LUN_FORMATTING 0x0404		// LUN format in progress
#define SCSIS_ASC_LUN_SEL_FAILED 0x0500		// LUN doesn't respond to selection
#define SCSIS_ASC_LUN_COM_FAILURE 0x0800	// LUN communication failure
#define SCSIS_ASC_LUN_TIMEOUT 0x0801		// LUN communication time-out
#define SCSIS_ASC_LUN_COM_PARITY 0x0802		// LUN communication parity failure
#define SCSIS_ASC_LUN_COM_CRC 0x0803		// LUN communication CRC failure (SCSI-3)
#define SCSIS_ASC_WRITE_ERR_AUTOREALLOC 0x0c01	// recovered by auto-reallocation
#define SCSIS_ASC_WRITE_ERR_AUTOREALLOC_FAILED 0x0c02
#define SCSIS_ASC_ECC_ERROR 0x1000
#define SCSIS_ASC_UNREC_READ_ERR 0x1100		// unrecovered read error
#define SCSIS_ASC_READ_RETRIES_EXH 0x1101	// read retries exhausted
#define SCSIS_ASC_UNREC_READ_ERR_AUTOREALLOC_FAILED 0x1104 // above + auto-reallocate failed
#define SCSIS_ASC_RECORD_NOT_FOUND 0x1401
#define SCSIS_ASC_RANDOM_POS_ERROR 0x1500	// random positioning error
#define SCSIS_ASC_POSITIONING_ERR 0x1501	// mechanical positioning error
#define SCSIS_ASC_POS_ERR_ON_READ 0x1502	// positioning error detected by reading
#define SCSIS_ASC_DATA_RECOV_NO_ERR_CORR 0x1700	// recovered with no error correction applied
#define SCSIS_ASC_DATA_RECOV_WITH_RETRIES 0x1701 
#define SCSIS_ASC_DATA_RECOV_POS_HEAD_OFS 0x1702 // ?recovered with positive head offset
#define SCSIS_ASC_DATA_RECOV_NEG_HEAD_OFS 0x1703 // ?recovered with negative head offset
#define SCSIS_ASC_DATA_RECOV_WITH_RETRIES_CIRC 0x1704 // recovered with retries/CIRC
#define SCSIS_ASC_DATA_RECOV_PREV_SECT_ID 0x1705 // recovered using previous sector ID
#define SCSIS_ASC_DATA_RECOV_NO_ECC_AUTOREALLOC 0x1706
#define SCSIS_ASC_DATA_RECOV_NO_ECC_REASSIGN 0x1707 // reassign recommended
#define SCSIS_ASC_DATA_RECOV_NO_ECC_REWRITE 0x1708 // rewrite recommended
#define SCSIS_ASC_DATA_RECOV_WITH_CORR 0x1800 // recovered using error correction
#define SCSIS_ASC_DATA_RECOV_WITH_CORR_RETRIES 0x1801 // used error correction and retries
#define SCSIS_ASC_DATA_RECOV_AUTOREALLOC 0x1802
#define SCSIS_ASC_DATA_RECOV_CIRC 0x1803	// recovered using CIRC
#define SCSIS_ASC_DATA_RECOV_LEC 0x1804		// recovered using LEC
#define SCSIS_ASC_DATA_RECOV_REASSIGN 0x1805 // reassign recommended
#define SCSIS_ASC_DATA_RECOV_REWRITE 0x1806 // rewrite recommended
#define SCSIS_ASC_PARAM_LIST_LENGTH_ERR 0x1a00	// parameter list too short
#define SCSIS_ASC_ID_RECOV 0x1e00			// recoved ID with ECC
#define SCSIS_ASC_INV_OPCODE 0x2000
#define SCSIS_ASC_LBA_OOR 0x2100			// LBA out of range
#define SCSIS_ASC_ILL_FUNCTION 0x2200		// better use 0x2000/0x2400/0x2600 instead
#define SCSIS_ASC_INV_CDB_FIELD 0x2400
#define SCSIS_ASC_LUN_NOT_SUPPORTED 0x2500
#define SCSIS_ASC_INV_PARAM_LIST_FIELD 0x2600
#define SCSIS_ASC_PARAM_NOT_SUPPORTED 0x2601
#define SCSIS_ASC_PARAM_VALUE_INV 0x2602
#define SCSIS_ASC_WRITE_PROTECTED 0x2700
#define SCSIS_ASC_MEDIUM_CHANGED 0x2800
#define SCSIS_ASC_WAS_RESET 0x2900			// reset by power-on/bus reset/device reset
#define SCSIS_ASC_PARAMS_CHANGED 0x2a00
#define SCSIS_ASC_MEDIUM_FORMAT_CORRUPTED 0x3100
#define SCSIS_ASC_ROUNDED_PARAM 0x3700		// parameter got rounded
#define SCSIS_ASC_NO_MEDIUM 0x3a00			// medium not present
#define SCSIS_ASC_INTERNAL_FAILURE 0x4400
#define SCSIS_ASC_SEL_FAILURE 0x4500		// select/reselect failure
#define SCSIS_ASC_UNSUCC_SOFT_RESET 0x4600	// unsuccessful soft reset
#define SCSIS_ASC_SCSI_PARITY_ERR 0x4700	// SCSI parity error
#define SCSIS_ASC_LOAD_EJECT_FAILED 0x5300	// media load or eject failed
#define SCSIS_ASC_REMOVAL_PREVENTED 0x5302	// medium removal prevented
#define SCSIS_ASC_REMOVAL_REQUESTED 0x5a01	// operator requests medium removal


// some scsi op-codes
#define	SCSI_OP_TUR 0x00
#define SCSI_OP_REQUEST_SENSE 0x03
#define SCSI_OP_FORMAT 0x04
#define	SCSI_OP_READ_6 0x08
#define SCSI_OP_WRITE_6 0x0a
#define SCSI_OP_INQUIRY 0x12
#define SCSI_OP_MODE_SELECT_6 0x15
#define SCSI_OP_RESERVE 0x16
#define SCSI_OP_RELEASE 0x17
#define SCSI_OP_MODE_SENSE_6 0x1a
#define SCSI_OP_START_STOP 0x1b
#define	SCSI_OP_RECEIVE_DIAGNOSTIC 0x1c
#define	SCSI_OP_SEND_DIAGNOSTIC 0x1d
#define SCSI_OP_PREVENT_ALLOW 0x1e
#define	SCSI_OP_READ_CAPACITY 0x25
#define	SCSI_OP_READ_10 0x28
#define SCSI_OP_WRITE_10 0x2a
#define SCSI_OP_POSITION_TO_ELEMENT 0x2b
#define SCSI_OP_VERIFY 0x2f
#define	SCSI_OP_SYNCHRONIZE_CACHE 0x35
#define	SCSI_OP_WRITE_BUFFER 0x3b
#define	SCSI_OP_READ_BUFFER 0x3c
#define	SCSI_OP_CHANGE_DEFINITION 0x40
#define	SCSI_OP_MODE_SELECT_10 0x55
#define	SCSI_OP_MODE_SENSE_10 0x5A
#define SCSI_OP_MOVE_MEDIUM 0xa5
#define SCSI_OP_READ_12 0xa8
#define SCSI_OP_WRITE_12 0xaa
#define SCSI_OP_READ_ELEMENT_STATUS 0xb8


// INQUIRY

typedef struct scsi_cmd_inquiry {
	uint8 opcode;
	LBITFIELD8_3(
		EVPD : 1,							// enhanced vital product data
		res1_1 : 4,
		LUN : 3
	);
	uint8 page_code;
	uint8 res3;
	uint8 allocation_length;
	uint8 control;
} scsi_cmd_inquiry;

typedef struct scsi_res_inquiry {
	LBITFIELD8_2(
		device_type : 5,
		device_qualifier : 3
	);
	LBITFIELD8_2(
		device_type_modifier : 7,			// obsolete, normally set to zero
		RMB : 1								// 1 = removable medium
	);
	LBITFIELD8_3(							// 0 always means "not conforming"
		ANSI_version : 3,					// 1 for SCSI-1, 2 for SCSI-2 etc.
		ECMA_version : 3,
		ISO_version : 2
	);
	LBITFIELD8_4(
		response_data_format : 4,			// 2 = SCSI/2 compliant
		res3_4 : 2,
		TrmIOP : 1,							// 1 = supports TERMINATE I/O PROCESS
		AENC : 1							// processor devices only : 
											// Asynchronous Event Notification Capable
	);
	uint8 additional_length;				// total (whished) length = this + 4
	uint8 res5;
	uint8 res6;
	LBITFIELD8_8(
		SftRe : 1,							// 0 = soft reset leads to hard reset
		CmdQue : 1,							// 1 = supports tagged command queuing
		res7_2 : 1,
		Linked : 1,							// 1 = supports linked commands
		Sync : 1,							// 1 = supports synchronous transfers
		WBus16 : 1,							// 1 = supports 16 bit transfers
		WBus32 : 1,							// 1 = supports 32 bit transfers
		RelAdr : 1							// 1 = supports relative addr. for linking
	);
	char vendor_ident[8];					// 8
	char product_ident[16];					// 16
	char product_rev[4];					// 32
	
	// XPT doesn't return following data on XPT_GDEV_TYPE
	uint8 vendor_spec[30];					// 36
	uint8 res56[40];						// 56
	/* additional vendor specific data */	// 96
} scsi_res_inquiry;

enum {
	scsi_periph_qual_connected = 0,
	scsi_periph_qual_not_connected = 2,
	scsi_periph_qual_not_connectable = 3
	// value 1 is reserved, values of 4 and above are vendor-specific
} scsi_peripheral_qualifier;

enum {
	scsi_dev_direct_access = 0,
	scsi_dev_sequential_access = 1,
	scsi_dev_printer = 2,
	scsi_dev_processor = 3,
	scsi_dev_WOM = 4,
	scsi_dev_CDROM = 5,
	scsi_dev_scanner = 6,
	scsi_dev_optical = 7,
	scsi_dev_medium_changer = 8,
	scsi_dev_communication = 9,
	// 0xa - 0xb are graphics arts pre-press devices
	// 0xc - 0x1e reserved
	scsi_dev_unknown = 0x1f 	// used for scsi_periph_qual_not_connectable
} scsi_device_type;


// vital product data: unit serial number page

#define SCSI_PAGE_USN 0x80

typedef struct scsi_page_USN {
	LBITFIELD8_2(
		device_type : 5,
		device_qualifier : 3
	);
	uint8 page_code;
	uint8 res2;
	
	uint8 page_length;		// total size = this + 3
	char PSN[1];			// size according to page_length
} scsi_page_USN;

// READ CAPACITY

typedef struct scsi_cmd_read_capacity {
	uint8 opcode;
	LBITFIELD8_3(
		RelAdr : 1,							// relative address
		res1_1 : 4,
		LUN : 3
	);
	uint8 top_LBA;
	uint8 high_LBA;
	uint8 mid_LBA;
	uint8 low_LBA;
	uint8 res6[2];
	LBITFIELD8_2(
		PMI : 1,							// partial medium indicator
		res8_1 : 7
	);
	uint8 control;
} scsi_cmd_read_capacity;

typedef struct scsi_res_read_capacity {
	uint8 top_LBA;
	uint8 high_LBA;
	uint8 mid_LBA;
	uint8 low_LBA;
	uint8 top_block_size;					// in bytes
	uint8 high_block_size;
	uint8 mid_block_size;
	uint8 low_block_size;
} scsi_res_read_capacity;


// READ (6), WRITE (6)

typedef struct scsi_cmd_rw_6 {
	uint8 opcode;
	LBITFIELD8_2(
		high_LBA : 5,
		LUN : 3
	);
	uint8 mid_LBA;
	uint8 low_LBA;
	uint8 length;							// 0 = 256 blocks
	uint8 control;
} scsi_cmd_rw_6;


// READ (10), WRITE (10)

typedef struct scsi_cmd_rw_10 {
	uint8 opcode;
	LBITFIELD8_5(
		RelAdr : 1,							// relative address
		res1_1 : 2,
		FUA : 1,							// force unit access (1 = safe, cacheless access)
		DPO : 1,							// disable page out (1 = not worth caching)
		LUN : 3
	);
	uint8 top_LBA;
	uint8 high_LBA;
	uint8 mid_LBA;
	uint8 low_LBA;
	uint8 res6;
	uint8 high_length;						// 0 = no block
	uint8 low_length;
	uint8 control;
} scsi_cmd_rw_10;


// READ (12), WRITE (12)

typedef struct scsi_cmd_rw_12 {
	uint8 opcode;
	LBITFIELD8_5(
		RelAdr : 1,							// relative address
		res1_1 : 2,
		FUA : 1,							// force unit access (1 = safe, cacheless access)
		DPO : 1,							// disable page out (1 = not worth caching)
		LUN : 3
	);
	uint8 top_LBA;
	uint8 high_LBA;
	uint8 mid_LBA;
	uint8 low_LBA;
	uint8 top_length;						// 0 = no block
	uint8 high_length;
	uint8 mid_length;
	uint8 low_length;
	uint8 res10;
	uint8 control;
} scsi_cmd_rw_12;


// REQUEST SENSE

typedef struct scsi_cmd_request_sense {
	uint8 opcode;
	LBITFIELD8_2(
		res1_0 : 5,
		LUN : 3
	);
	uint8 res2[2];
	uint8 alloc_length;
	uint8 control;
} scsi_cmd_request_sense;


// sense data structures

#define SCSIS_CURR_ERROR 0x70
#define SCSIS_DEFERRED_ERROR 0x71

typedef struct scsi_sense {
	LBITFIELD8_2(
		error_code : 7,
		valid : 1							// 0 = not conforming to standard
	);
	uint8 segment_number;					// for COPY/COPY AND VERIFY/COMPARE
	LBITFIELD8_5(
		sense_key : 4,
		res2_4 : 1,
		ILI : 1,							// incorrect length indicator - req. block 
											// length doesn't match physical block length
		EOM : 1,							// serial devices only
		Filemark : 1						// optional for random access
	);
	
	uint8 highest_inf;						// device-type or command specific
	uint8 high_inf;							// device-type 0, 4, 5, 7: block address
	uint8 mid_inf;							// device-type 1, 2, 3: req length - act. length
	uint8 low_inf;							// (and others for sequential dev. and COPY cmds
	
	uint8 add_sense_length; 				// total length = this + 7
	
	uint8 highest_cmd_inf;
	uint8 high_cmd_inf;
	uint8 mid_cmd_inf;
	uint8 low_cmd_inf;
	uint8 asc;
	uint8 ascq;								// this can be zero if unsupported
	uint8 unit_code;						// != 0 to specify internal device unit
	
	union {
		struct {
		LBITFIELD8_2(
			high_key_spec : 7,
			SKSV : 1						// 1 = sense key specific (byte 15-17) valid
		);
		uint8 mid_key_spec;
		uint8 low_key_spec;
		} raw;
		
		// ILLEGAL REQUEST
		struct {
		LBITFIELD8_5(
			bit_pointer : 3,				// points to (highest) invalid bit of parameter
			BPV : 1,						// 1 = bit_pointer is valid
			res15_4 : 2,
			c_d : 2,						// 1 = error command, 0 = error in data
			SKSV : 1						// s.a.
		);
		uint8 high_field_pointer;			// points to (highest) invalid byte of parameter
		uint8 low_field_pointer;			// (!using big endian, this means the first byte!)
		} ill_request;

		// access error (RECOVERED, HARDWARE or MEDIUM ERROR)
		struct {
		LBITFIELD8_2(
			res15_0 : 7,
			SKSV : 1	
		);
		uint8 high_retry_cnt;
		uint8 low_retry_cnt;
		} acc_error;
		
		// format progress (if sense key = NOT READY)
		struct {
		LBITFIELD8_2(
			res15_0 : 7,
			SKSV : 1	
		);
		uint8 high_progress;				// 0 = start, 0xffff = almost finished
		uint8 low_progress;
		} format_progress;
	} sense_key_spec;
		
	// starting with offset 18 there are additional sense byte
} scsi_sense;


// PREVENT ALLOW

typedef struct scsi_cmd_prevent_allow {
	uint8 opcode;
	LBITFIELD8_2(
		res1_0 : 5,
		LUN : 3
	);
	uint8 res2[2];
	LBITFIELD8_2(
		prevent : 1,
		res4_1 : 7
	);
	uint8 control;
} scsi_cmd_prevent_allow;

// START STOP UNIT

typedef struct scsi_cmd_ssu {
	uint8 opcode;
	LBITFIELD8_3(
		immed : 1,			// 1 - return immediately, 0 - waiting until completed
		res1_1 : 4,
		LUN : 3
	);
	uint8 res2[2];
	LBITFIELD8_3(
		start : 1,			// 1 - load+start, i.e. allow, 0 - eject+stop, i.e. deny
		LoEj : 1,			// 1 - include loading/ejecting, 0 - only to allow/deny
		res4_2 : 6
	);
	uint8 control;
} scsi_cmd_ssu;


// MODE SELECT (6)

typedef struct scsi_cmd_mode_select_6 {
	uint8 opcode;
	LBITFIELD8_4(
		SP : 1,				// 1 = save pages to non-volatile memory
		res1_1 : 3,
		PF : 1,				// 0 = old SCSI-1; 1 = new SCSI-2 format
		LUN : 3
	);
	uint8 res2[2];
	uint8 param_list_length;	// data size
	uint8 control;
} scsi_cmd_mode_select_6;


// MODE SENSE (6)

typedef struct scsi_cmd_mode_sense_6 {
	uint8 opcode;
	LBITFIELD8_4(
		res1_0 : 3,
		DBD : 1,			// disable block descriptors
		res1_4 : 1,
		LUN : 3
	);
	LBITFIELD8_2(
		page_code : 6,
		PC : 2				// page control field
	);
	uint8 res3;
	uint8 allocation_length;	// maximum amount of data
	uint8 control;
} scsi_cmd_mode_sense_6;


// MODE SELECT (10)

typedef struct scsi_cmd_mode_select_10 {
	uint8 opcode;
	LBITFIELD8_4(
		SP : 1,				// 1 = save pages to non-volatile memory
		res1_1 : 3,
		PF : 1,				// 0 = old SCSI-1; 1 = new SCSI-2 format
		LUN : 3
	);
	uint8 res2[5];
	uint8 high_param_list_length;	// data size
	uint8 low_param_list_length;
	uint8 control;
} scsi_cmd_mode_select_10;


// MODE SENSE (10)

typedef struct scsi_cmd_mode_sense_10 {
	uint8 opcode;
	LBITFIELD8_4(
		res1_0 : 3,
		DBD : 1,			// disable block descriptors
		res1_4 : 1,
		LUN : 3
	);
	LBITFIELD8_2(
		page_code : 6,
		PC : 2				// page control field
	);
	uint8 res3[4];
	uint8 high_allocation_length;	// maximum amount of data
	uint8 low_allocation_length;
	uint8 control;
} scsi_cmd_mode_sense_10;


// possible contents of PC
#define SCSI_MODE_SENE_PC_CURRENT 0
#define SCSI_MODE_SENE_PC_CHANGABLE 1		// changable field are filled with "1"
#define SCSI_MODE_SENE_PC_DEFAULT 2
#define SCSI_MODE_SENE_PC_SAVED 3

// special mode page indicating to return all mode pages
#define SCSI_MODEPAGE_ALL 0x3f

// header of mode data; followed by block descriptors and mode pages
typedef struct scsi_mode_param_header_6 {
	uint8 mode_data_len;		// total length excluding this byte
	uint8 medium_type;
	uint8 dev_spec_parameter;
	uint8 block_desc_len;		// total length of all transmitted block descriptors
} scsi_mode_param_header_6;

typedef struct scsi_mode_param_header_10 {
	uint8 high_mode_data_len;	// total length excluding this byte
	uint8 low_mode_data_len;
	uint8 medium_type;
	uint8 dev_spec_parameter;
	uint8 res4[2];
	uint8 high_block_desc_len;	// total length of all transmitted block descriptors
	uint8 low_block_desc_len;
} scsi_mode_param_header_10;


// content of dev_spec_parameter for direct access devices
typedef struct scsi_mode_param_dev_spec_da {
	LBITFIELD8_4(
		res0_0 : 4,
		DPOFUA : 1,				// 1 = supports DPO and FUA, see READ (10) (sense only)
		res0_6 : 1,
		WP : 1					// write protected (sense only)
	);
} scsi_mode_param_dev_spec_da;

typedef struct scsi_mode_param_block_desc {
	uint8 density;				// density code of area
	uint8 high_numblocks;		// size of this area in blocks
	uint8 med_numblocks;		// 0 = all remaining blocks
	uint8 low_numblocks;
	uint8 res4;
	uint8 high_blocklen;		// block size
	uint8 med_blocklen;
	uint8 low_blocklen;
} scsi_mode_param_block_desc;


// header of a mode pages
typedef struct scsi_modepage_header {
	LBITFIELD8_3(
		page_code : 6,
		res0_6 : 1,
		PS : 1				// 1 = page can be saved (only valid for MODE SENSE)
	);
	uint8 page_length;		// size of page excluding this common header
} scsi_modepage_header;


// control mode page
#define SCSI_MODEPAGE_CONTROL 0xa

typedef struct scsi_modepage_contr {
	scsi_modepage_header header;
	LBITFIELD8_2(
		RLEC : 1,			// Report Log Exception Condition
		res2_1 : 7
	);
	LBITFIELD8_4(
		DQue : 1,			// disable Queuing
		QErr : 1,			// abort queued commands on contingent allegiance condition
		res3_2 : 2,
		QAM : 4				// Queue Algorithm Modifier
	);
	LBITFIELD8_5(
		EAENP : 1,			// error AEN permission; true = send AEN on deferred error
							// false = generate UA condition after deferred error
		UAAENP : 1,			// unit attention AEN permission; true = send AEN,
							// false = generate UA condition (for everything but init.)
		RAENP : 1,			// ready AEN permission; true = send async event notification
							// (AEN) instead of generating an Unit Attention (UA) Condition 
							// after initialization
		res4_3 : 4,
		EECA : 1			// enable Extended Contingent Allegiance
	);
	uint8 res5;
	uint8 high_AEN_holdoff;	// ready AEN hold off period - delay in ms between 
	uint8 low_AEN_holdoff;	// initialization and AEN
} scsi_modepage_contr;

// values for QAM
#define SCSI_QAM_RESTRICTED 0
#define SCSI_QAM_UNRESTRICTED 1
// 2 - 7 reserved, 8 - 0xf vendor-specific


// TUR

typedef struct scsi_cmd_tur {
	uint8 opcode;
	LBITFIELD8_2(
		res1_0 : 5,
		LUN : 3
	);
	uint8 res3[3];
	uint8 control;
} scsi_cmd_tur;

#endif
