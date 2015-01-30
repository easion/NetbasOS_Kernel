
#include "../netbas.h"
#include <usb_port.h>
#include <usb.h>
#include <usb_subr.h>
#include <usbdevs.h>
#include "umass.h"

//dpp
#define	UE_ADDR_ANY	0xff		/* for internal use only! */
#define	UE_DIR_ANY	0xff		/* for internal use only! */


#if 0
static const struct umass_devdescr umass_devdescr[] = {
	{USB_VENDOR_ASAHIOPTICAL, PID_WILDCARD, RID_WILDCARD,
		UMASS_PROTO_ATAPI | UMASS_PROTO_CBI_I,
		RS_NO_CLEAR_UA
	},
	{USB_VENDOR_ADDON, USB_PRODUCT_ADDON_ATTACHE, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		IGNORE_RESIDUE
	},
	{USB_VENDOR_ADDON, USB_PRODUCT_ADDON_A256MB, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		IGNORE_RESIDUE
	},
	{USB_VENDOR_ADDON, USB_PRODUCT_ADDON_DISKPRO512, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		IGNORE_RESIDUE
	},
	{USB_VENDOR_ADDONICS2, USB_PRODUCT_ADDONICS2_CABLE_205, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		NO_QUIRKS
	},
	{USB_VENDOR_AIPTEK, USB_PRODUCT_AIPTEK_POCKETCAM3M, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		NO_QUIRKS
	},
	{USB_VENDOR_ALCOR, USB_PRODUCT_ALCOR_UMCR_9361, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		NO_GETMAXLUN
	},
	{USB_VENDOR_ASAHIOPTICAL, USB_PRODUCT_ASAHIOPTICAL_OPTIO230, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		NO_INQUIRY
	},
	{USB_VENDOR_ASAHIOPTICAL, USB_PRODUCT_ASAHIOPTICAL_OPTIO330, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		NO_INQUIRY
	},
	{USB_VENDOR_BELKIN, USB_PRODUCT_BELKIN_USB2SCSI, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		NO_QUIRKS
	},
	{USB_VENDOR_CASIO, USB_PRODUCT_CASIO_QV_DIGICAM, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_CBI,
		NO_INQUIRY
	},
	{USB_VENDOR_CCYU, USB_PRODUCT_CCYU_ED1064, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		NO_QUIRKS
	},
	{USB_VENDOR_CENTURY, USB_PRODUCT_CENTURY_EX35QUAT, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		FORCE_SHORT_INQUIRY | NO_START_STOP | IGNORE_RESIDUE
	},
	{USB_VENDOR_DESKNOTE, USB_PRODUCT_DESKNOTE_UCR_61S2B, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		NO_QUIRKS
	},
	{USB_VENDOR_DMI, USB_PRODUCT_DMI_CFSM_RW, RID_WILDCARD,
		UMASS_PROTO_SCSI,
		NO_GETMAXLUN
	},
	{USB_VENDOR_EPSON, USB_PRODUCT_EPSON_STYLUS_875DC, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_CBI,
		NO_INQUIRY
	},
	{USB_VENDOR_EPSON, USB_PRODUCT_EPSON_STYLUS_895, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		NO_GETMAXLUN
	},
	{USB_VENDOR_FEIYA, USB_PRODUCT_FEIYA_5IN1, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		NO_QUIRKS
	},
	{USB_VENDOR_FREECOM, USB_PRODUCT_FREECOM_DVD, RID_WILDCARD,
		UMASS_PROTO_SCSI,
		NO_QUIRKS
	},
	{USB_VENDOR_FUJIPHOTO, USB_PRODUCT_FUJIPHOTO_MASS0100, RID_WILDCARD,
		UMASS_PROTO_ATAPI | UMASS_PROTO_CBI_I,
		RS_NO_CLEAR_UA
	},
	{USB_VENDOR_GENESYS, USB_PRODUCT_GENESYS_GL641USB2IDE, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		FORCE_SHORT_INQUIRY | NO_START_STOP | IGNORE_RESIDUE
	},
	{USB_VENDOR_GENESYS, USB_PRODUCT_GENESYS_GL641USB2IDE_2, RID_WILDCARD,
		UMASS_PROTO_ATAPI | UMASS_PROTO_BBB,
		FORCE_SHORT_INQUIRY | NO_START_STOP | IGNORE_RESIDUE
	},
	{USB_VENDOR_GENESYS, USB_PRODUCT_GENESYS_GL641USB, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		FORCE_SHORT_INQUIRY | NO_START_STOP | IGNORE_RESIDUE
	},
	{USB_VENDOR_GENESYS, USB_PRODUCT_GENESYS_GL641USB_2, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		WRONG_CSWSIG
	},
	{USB_VENDOR_HAGIWARA, USB_PRODUCT_HAGIWARA_FG, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		NO_QUIRKS
	},
	{USB_VENDOR_HAGIWARA, USB_PRODUCT_HAGIWARA_FGSM, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		NO_QUIRKS
	},
	{USB_VENDOR_HITACHI, USB_PRODUCT_HITACHI_DVDCAM_DZ_MV100A, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_CBI,
		NO_GETMAXLUN
	},
	{USB_VENDOR_HITACHI, USB_PRODUCT_HITACHI_DVDCAM_USB, RID_WILDCARD,
		UMASS_PROTO_ATAPI | UMASS_PROTO_CBI_I,
		NO_INQUIRY
	},
	{USB_VENDOR_HP, USB_PRODUCT_HP_CDW4E, RID_WILDCARD,
		UMASS_PROTO_ATAPI,
		NO_QUIRKS
	},
	{USB_VENDOR_HP, USB_PRODUCT_HP_CDW8200, RID_WILDCARD,
		UMASS_PROTO_ATAPI | UMASS_PROTO_CBI_I,
		NO_TEST_UNIT_READY | NO_START_STOP
	},
	{USB_VENDOR_IMAGINATION, USB_PRODUCT_IMAGINATION_DBX1, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		WRONG_CSWSIG
	},
	{USB_VENDOR_INSYSTEM, USB_PRODUCT_INSYSTEM_USBCABLE, RID_WILDCARD,
		UMASS_PROTO_ATAPI | UMASS_PROTO_CBI,
		NO_TEST_UNIT_READY | NO_START_STOP | ALT_IFACE_1
	},
	{USB_VENDOR_INSYSTEM, USB_PRODUCT_INSYSTEM_ATAPI, RID_WILDCARD,
		UMASS_PROTO_RBC | UMASS_PROTO_CBI,
		NO_QUIRKS
	},
	{USB_VENDOR_INSYSTEM, USB_PRODUCT_INSYSTEM_STORAGE_V2, RID_WILDCARD,
		UMASS_PROTO_RBC | UMASS_PROTO_CBI,
		NO_QUIRKS
	},
	{USB_VENDOR_IODATA, USB_PRODUCT_IODATA_IU_CD2, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		NO_QUIRKS
	},
	{USB_VENDOR_IODATA, USB_PRODUCT_IODATA_DVR_UEH8, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		NO_QUIRKS
	},
	{USB_VENDOR_IOMEGA, USB_PRODUCT_IOMEGA_ZIP100, RID_WILDCARD,
		/*
		 * XXX This is not correct as there are Zip drives that use
		 * ATAPI.
		 */
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		NO_TEST_UNIT_READY
	},
	{USB_VENDOR_KYOCERA, USB_PRODUCT_KYOCERA_FINECAM_L3, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		NO_INQUIRY
	},
	{USB_VENDOR_KYOCERA, USB_PRODUCT_KYOCERA_FINECAM_S3X, RID_WILDCARD,
		UMASS_PROTO_ATAPI | UMASS_PROTO_CBI,
		NO_INQUIRY
	},
	{USB_VENDOR_KYOCERA, USB_PRODUCT_KYOCERA_FINECAM_S4, RID_WILDCARD,
		UMASS_PROTO_ATAPI | UMASS_PROTO_CBI,
		NO_INQUIRY
	},
	{USB_VENDOR_KYOCERA, USB_PRODUCT_KYOCERA_FINECAM_S5, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		NO_INQUIRY
	},
	{USB_VENDOR_LACIE, USB_PRODUCT_LACIE_HD, RID_WILDCARD,
		UMASS_PROTO_RBC | UMASS_PROTO_CBI,
		NO_QUIRKS
	},
	{USB_VENDOR_LEXAR, USB_PRODUCT_LEXAR_CF_READER, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		NO_INQUIRY
	},
	{USB_VENDOR_LEXAR, USB_PRODUCT_LEXAR_JUMPSHOT, RID_WILDCARD,
		UMASS_PROTO_SCSI,
		NO_QUIRKS
	},
	{USB_VENDOR_LOGITEC, USB_PRODUCT_LOGITEC_LDR_H443SU2, RID_WILDCARD,
		UMASS_PROTO_SCSI,
		NO_QUIRKS
	},
	{USB_VENDOR_LOGITEC, USB_PRODUCT_LOGITEC_LDR_H443U2, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		NO_QUIRKS
	},
	{USB_VENDOR_MELCO, USB_PRODUCT_MELCO_DUBPXXG, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		FORCE_SHORT_INQUIRY | NO_START_STOP | IGNORE_RESIDUE
	},
	{USB_VENDOR_MICROTECH, USB_PRODUCT_MICROTECH_DPCM, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_CBI,
		NO_TEST_UNIT_READY | NO_START_STOP
	},
	{USB_VENDOR_MICROTECH, USB_PRODUCT_MICROTECH_SCSIDB25, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		NO_QUIRKS
	},
	{USB_VENDOR_MICROTECH, USB_PRODUCT_MICROTECH_SCSIHD50, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		NO_QUIRKS
	},
	{USB_VENDOR_MINOLTA, USB_PRODUCT_MINOLTA_E223, RID_WILDCARD,
		UMASS_PROTO_SCSI,
		NO_QUIRKS
	},
	{USB_VENDOR_MINOLTA, USB_PRODUCT_MINOLTA_F300, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		NO_QUIRKS
	},
	{USB_VENDOR_MITSUMI, USB_PRODUCT_MITSUMI_CDRRW, RID_WILDCARD,
		UMASS_PROTO_ATAPI | UMASS_PROTO_CBI,
		NO_QUIRKS
	},
	{USB_VENDOR_MITSUMI, USB_PRODUCT_MITSUMI_FDD, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		NO_GETMAXLUN
	},
	{USB_VENDOR_MOTOROLA2, USB_PRODUCT_MOTOROLA2_E398, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		FORCE_SHORT_INQUIRY | NO_INQUIRY_EVPD | NO_GETMAXLUN
	},
	{USB_VENDOR_MSYSTEMS, USB_PRODUCT_MSYSTEMS_DISKONKEY, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		IGNORE_RESIDUE | NO_GETMAXLUN | RS_NO_CLEAR_UA
	},
	{USB_VENDOR_MSYSTEMS, USB_PRODUCT_MSYSTEMS_DISKONKEY2, RID_WILDCARD,
		UMASS_PROTO_ATAPI | UMASS_PROTO_BBB,
		NO_QUIRKS
	},
	{USB_VENDOR_MYSON, USB_PRODUCT_MYSON_HEDEN, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		NO_INQUIRY | IGNORE_RESIDUE
	},
	{USB_VENDOR_NEODIO, USB_PRODUCT_NEODIO_ND3260, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		FORCE_SHORT_INQUIRY
	},
	{USB_VENDOR_NETAC, USB_PRODUCT_NETAC_CF_CARD, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		NO_INQUIRY
	},
	{USB_VENDOR_NETAC, USB_PRODUCT_NETAC_ONLYDISK, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		IGNORE_RESIDUE
	},
	{USB_VENDOR_NETCHIP, USB_PRODUCT_NETCHIP_CLIK_40, RID_WILDCARD,
		UMASS_PROTO_ATAPI,
		NO_INQUIRY
	},
	{USB_VENDOR_NIKON, USB_PRODUCT_NIKON_D300, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		NO_QUIRKS
	},
	{USB_VENDOR_OLYMPUS, USB_PRODUCT_OLYMPUS_C1, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		WRONG_CSWSIG
	},
	{USB_VENDOR_OLYMPUS, USB_PRODUCT_OLYMPUS_C700, RID_WILDCARD,
		UMASS_PROTO_SCSI,
		NO_GETMAXLUN
	},
	{USB_VENDOR_ONSPEC, USB_PRODUCT_ONSPEC_CFMS_RW, RID_WILDCARD,
		UMASS_PROTO_SCSI,
		NO_QUIRKS
	},
	{USB_VENDOR_ONSPEC, USB_PRODUCT_ONSPEC_CFSM_COMBO, RID_WILDCARD,
		UMASS_PROTO_SCSI,
		NO_QUIRKS
	},
	{USB_VENDOR_ONSPEC, USB_PRODUCT_ONSPEC_CFSM_READER, RID_WILDCARD,
		UMASS_PROTO_SCSI,
		NO_QUIRKS
	},
	{USB_VENDOR_ONSPEC, USB_PRODUCT_ONSPEC_CFSM_READER2, RID_WILDCARD,
		UMASS_PROTO_SCSI,
		NO_QUIRKS
	},
	{USB_VENDOR_ONSPEC, USB_PRODUCT_ONSPEC_MDCFE_B_CF_READER, RID_WILDCARD,
		UMASS_PROTO_SCSI,
		NO_QUIRKS
	},
	{USB_VENDOR_ONSPEC, USB_PRODUCT_ONSPEC_MDSM_B_READER, RID_WILDCARD,
		UMASS_PROTO_SCSI,
		NO_INQUIRY
	},
	{USB_VENDOR_ONSPEC, USB_PRODUCT_ONSPEC_READER, RID_WILDCARD,
		UMASS_PROTO_SCSI,
		NO_QUIRKS
	},
	{USB_VENDOR_ONSPEC, USB_PRODUCT_ONSPEC_UCF100, RID_WILDCARD,
		UMASS_PROTO_ATAPI | UMASS_PROTO_BBB,
		NO_INQUIRY | NO_GETMAXLUN
	},
	{USB_VENDOR_ONSPEC2, USB_PRODUCT_ONSPEC2_IMAGEMATE_SDDR55, RID_WILDCARD,
		UMASS_PROTO_SCSI,
		NO_GETMAXLUN
	},
	{USB_VENDOR_PANASONIC, USB_PRODUCT_PANASONIC_KXL840AN, RID_WILDCARD,
		UMASS_PROTO_ATAPI | UMASS_PROTO_BBB,
		NO_GETMAXLUN
	},
	{USB_VENDOR_PANASONIC, USB_PRODUCT_PANASONIC_KXLCB20AN, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		NO_QUIRKS
	},
	{USB_VENDOR_PANASONIC, USB_PRODUCT_PANASONIC_KXLCB35AN, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		NO_QUIRKS
	},
	{USB_VENDOR_PANASONIC, USB_PRODUCT_PANASONIC_LS120CAM, RID_WILDCARD,
		UMASS_PROTO_UFI,
		NO_QUIRKS
	},
	{USB_VENDOR_PLEXTOR, USB_PRODUCT_PLEXTOR_40_12_40U, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		NO_TEST_UNIT_READY
	},
	{USB_VENDOR_PNY, USB_PRODUCT_PNY_ATTACHE2, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		IGNORE_RESIDUE | NO_START_STOP
	},
	{USB_VENDOR_SAMSUNG, USB_PRODUCT_SAMSUNG_YP_U2, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		SHUTTLE_INIT | NO_GETMAXLUN
	},
	{USB_VENDOR_SAMSUNG_TECHWIN, USB_PRODUCT_SAMSUNG_TECHWIN_DIGIMAX_410, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		NO_INQUIRY
	},
	{USB_VENDOR_SANDISK, USB_PRODUCT_SANDISK_SDDR05A, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_CBI,
		READ_CAPACITY_OFFBY1 | NO_GETMAXLUN
	},
	{USB_VENDOR_SANDISK, USB_PRODUCT_SANDISK_SDDR09, RID_WILDCARD,
		UMASS_PROTO_SCSI,
		READ_CAPACITY_OFFBY1 | NO_GETMAXLUN
	},
	{USB_VENDOR_SANDISK, USB_PRODUCT_SANDISK_SDDR12, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_CBI,
		READ_CAPACITY_OFFBY1 | NO_GETMAXLUN
	},
	{USB_VENDOR_SANDISK, USB_PRODUCT_SANDISK_SDCZ2_256, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		IGNORE_RESIDUE
	},
	{USB_VENDOR_SANDISK, USB_PRODUCT_SANDISK_SDCZ4_128, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		IGNORE_RESIDUE
	},
	{USB_VENDOR_SANDISK, USB_PRODUCT_SANDISK_SDCZ4_256, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		IGNORE_RESIDUE
	},
	{USB_VENDOR_SANDISK, USB_PRODUCT_SANDISK_SDDR31, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		READ_CAPACITY_OFFBY1
	},
	{USB_VENDOR_SCANLOGIC, USB_PRODUCT_SCANLOGIC_SL11R, RID_WILDCARD,
		UMASS_PROTO_ATAPI | UMASS_PROTO_BBB,
		NO_INQUIRY
	},
	{USB_VENDOR_SHUTTLE, USB_PRODUCT_SHUTTLE_EUSB, RID_WILDCARD,
		UMASS_PROTO_ATAPI | UMASS_PROTO_CBI_I,
		NO_TEST_UNIT_READY | NO_START_STOP | SHUTTLE_INIT
	},
	{USB_VENDOR_SHUTTLE, USB_PRODUCT_SHUTTLE_CDRW, RID_WILDCARD,
		UMASS_PROTO_ATAPI | UMASS_PROTO_CBI,
		NO_QUIRKS
	},
	{USB_VENDOR_SHUTTLE, USB_PRODUCT_SHUTTLE_CF, RID_WILDCARD,
		UMASS_PROTO_ATAPI | UMASS_PROTO_CBI,
		NO_QUIRKS
	},
	{USB_VENDOR_SHUTTLE, USB_PRODUCT_SHUTTLE_EUSBATAPI, RID_WILDCARD,
		UMASS_PROTO_ATAPI | UMASS_PROTO_CBI,
		NO_QUIRKS
	},
	{USB_VENDOR_SHUTTLE, USB_PRODUCT_SHUTTLE_EUSBCFSM, RID_WILDCARD,
		UMASS_PROTO_SCSI,
		NO_QUIRKS
	},
	{USB_VENDOR_SHUTTLE, USB_PRODUCT_SHUTTLE_EUSCSI, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		NO_QUIRKS
	},
	{USB_VENDOR_SHUTTLE, USB_PRODUCT_SHUTTLE_HIFD, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_CBI,
		NO_GETMAXLUN
	},
	{USB_VENDOR_SHUTTLE, USB_PRODUCT_SHUTTLE_SDDR09, RID_WILDCARD,
		UMASS_PROTO_SCSI,
		NO_GETMAXLUN
	},
	{USB_VENDOR_SHUTTLE, USB_PRODUCT_SHUTTLE_ZIOMMC, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_CBI,
		NO_GETMAXLUN
	},
	{USB_VENDOR_SIGMATEL, USB_PRODUCT_SIGMATEL_I_BEAD100, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		SHUTTLE_INIT
	},
	{USB_VENDOR_SIIG, USB_PRODUCT_SIIG_WINTERREADER, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		IGNORE_RESIDUE
	},
	{USB_VENDOR_SKANHEX, USB_PRODUCT_SKANHEX_MD_7425, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		NO_INQUIRY
	},
	{USB_VENDOR_SKANHEX, USB_PRODUCT_SKANHEX_SX_520Z, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		NO_INQUIRY
	},
	{USB_VENDOR_SONY, USB_PRODUCT_SONY_HANDYCAM, 0x0500,
		UMASS_PROTO_RBC | UMASS_PROTO_CBI,
		RBC_PAD_TO_12
	},
	{USB_VENDOR_SONY, USB_PRODUCT_SONY_CLIE_40_MS, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		NO_INQUIRY
	},
	{USB_VENDOR_SONY, USB_PRODUCT_SONY_DSC, 0x0500,
		UMASS_PROTO_RBC | UMASS_PROTO_CBI,
		RBC_PAD_TO_12
	},
	{USB_VENDOR_SONY, USB_PRODUCT_SONY_DSC, 0x0600,
		UMASS_PROTO_RBC | UMASS_PROTO_CBI,
		RBC_PAD_TO_12
	},
	{USB_VENDOR_SONY, USB_PRODUCT_SONY_DSC, RID_WILDCARD,
		UMASS_PROTO_RBC | UMASS_PROTO_CBI,
		NO_QUIRKS
	},
	{USB_VENDOR_SONY, USB_PRODUCT_SONY_HANDYCAM, RID_WILDCARD,
		UMASS_PROTO_RBC | UMASS_PROTO_CBI,
		NO_QUIRKS
	},
	{USB_VENDOR_SONY, USB_PRODUCT_SONY_MSC, RID_WILDCARD,
		UMASS_PROTO_RBC | UMASS_PROTO_CBI,
		NO_QUIRKS
	},
	{USB_VENDOR_SONY, USB_PRODUCT_SONY_MS_MSC_U03, RID_WILDCARD,
		UMASS_PROTO_UFI | UMASS_PROTO_CBI,
		NO_GETMAXLUN
	},
	{USB_VENDOR_SONY, USB_PRODUCT_SONY_MS_NW_MS7, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		NO_GETMAXLUN
	},
	{USB_VENDOR_SONY, USB_PRODUCT_SONY_MS_PEG_N760C, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		NO_INQUIRY
	},
	{USB_VENDOR_SONY, USB_PRODUCT_SONY_MSACUS1, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		NO_GETMAXLUN
	},
	{USB_VENDOR_SONY, USB_PRODUCT_SONY_PORTABLE_HDD_V2, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		NO_QUIRKS
	},
	{USB_VENDOR_TAUGA, USB_PRODUCT_TAUGA_CAMERAMATE, RID_WILDCARD,
		UMASS_PROTO_SCSI,
		NO_QUIRKS
	},
	{USB_VENDOR_TEAC, USB_PRODUCT_TEAC_FD05PUB, RID_WILDCARD,
		UMASS_PROTO_UFI | UMASS_PROTO_CBI,
		NO_QUIRKS
	},
	{USB_VENDOR_TREK, USB_PRODUCT_TREK_MEMKEY, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		NO_INQUIRY
	},
	{USB_VENDOR_TREK, USB_PRODUCT_TREK_THUMBDRIVE_8MB, RID_WILDCARD,
		UMASS_PROTO_ATAPI | UMASS_PROTO_BBB,
		IGNORE_RESIDUE
	},
	{USB_VENDOR_TRUMPION, USB_PRODUCT_TRUMPION_C3310, RID_WILDCARD,
		UMASS_PROTO_UFI | UMASS_PROTO_CBI,
		NO_QUIRKS
	},
	{USB_VENDOR_TRUMPION, USB_PRODUCT_TRUMPION_MP3, RID_WILDCARD,
		UMASS_PROTO_RBC,
		NO_QUIRKS
	},
	{USB_VENDOR_TRUMPION, USB_PRODUCT_TRUMPION_T33520, RID_WILDCARD,
		UMASS_PROTO_SCSI,
		NO_QUIRKS
	},
	{USB_VENDOR_TWINMOS, USB_PRODUCT_TWINMOS_MDIV, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		NO_QUIRKS
	},
	{USB_VENDOR_VIA, USB_PRODUCT_VIA_USB2IDEBRIDGE, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		NO_SYNCHRONIZE_CACHE
	},
	{USB_VENDOR_VIVITAR, USB_PRODUCT_VIVITAR_35XX, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		NO_INQUIRY
	},
	{USB_VENDOR_WESTERN, USB_PRODUCT_WESTERN_COMBO, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		FORCE_SHORT_INQUIRY | NO_START_STOP | IGNORE_RESIDUE
	},
	{USB_VENDOR_WESTERN, USB_PRODUCT_WESTERN_EXTHDD, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		FORCE_SHORT_INQUIRY | NO_START_STOP | IGNORE_RESIDUE
	},
	{USB_VENDOR_WESTERN, USB_PRODUCT_WESTERN_MYBOOK, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		NO_INQUIRY_EVPD
	},
	{USB_VENDOR_WINMAXGROUP, USB_PRODUCT_WINMAXGROUP_FLASH64MC, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		NO_INQUIRY
	},
	{USB_VENDOR_YANO, USB_PRODUCT_YANO_FW800HD, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_BBB,
		FORCE_SHORT_INQUIRY | NO_START_STOP | IGNORE_RESIDUE
	},
	{USB_VENDOR_YANO, USB_PRODUCT_YANO_U640MO, RID_WILDCARD,
		UMASS_PROTO_ATAPI | UMASS_PROTO_CBI_I,
		FORCE_SHORT_INQUIRY
	},
	{USB_VENDOR_YEDATA, USB_PRODUCT_YEDATA_FLASHBUSTERU, RID_WILDCARD,
		UMASS_PROTO_SCSI | UMASS_PROTO_CBI,
		NO_GETMAXLUN
	},
	{USB_VENDOR_ZORAN, USB_PRODUCT_ZORAN_EX20DSC, RID_WILDCARD,
		UMASS_PROTO_ATAPI | UMASS_PROTO_CBI,
		NO_QUIRKS
	},
	{VID_EOT, PID_EOT, RID_EOT, 0, 0}
};
#else
static const struct umass_devdescr umass_devdescr[2] ;
#endif


/* prototypes */
int umass_probe(device_t dev);

int umass_attach(device_t dev);
int umass_detach(device_t dev);
//static device_probe_t umass_probe;
//static device_attach_t umass_attach;
//static device_detach_t umass_detach;

usb2_callback_t umass_tr_error;
usb2_callback_t umass_t_bbb_reset1_callback;
usb2_callback_t umass_t_bbb_reset2_callback;
usb2_callback_t umass_t_bbb_reset3_callback;
usb2_callback_t umass_t_bbb_command_callback;
usb2_callback_t umass_t_bbb_data_read_callback;
usb2_callback_t umass_t_bbb_data_rd_cs_callback;
usb2_callback_t umass_t_bbb_data_write_callback;
usb2_callback_t umass_t_bbb_data_wr_cs_callback;
usb2_callback_t umass_t_bbb_status_callback;
usb2_callback_t umass_t_cbi_reset1_callback;
usb2_callback_t umass_t_cbi_reset2_callback;
usb2_callback_t umass_t_cbi_reset3_callback;
usb2_callback_t umass_t_cbi_reset4_callback;
usb2_callback_t umass_t_cbi_command_callback;
usb2_callback_t umass_t_cbi_data_read_callback;
usb2_callback_t umass_t_cbi_data_rd_cs_callback;
usb2_callback_t umass_t_cbi_data_write_callback;
usb2_callback_t umass_t_cbi_data_wr_cs_callback;
usb2_callback_t umass_t_cbi_status_callback;

static void umass_cancel_ccb(struct umass_softc *sc);
static void umass_init_shuttle(struct umass_softc *sc);
static void umass_reset(struct umass_softc *sc);
static void umass_t_bbb_data_clear_stall_callback(struct usbd_xfer *xfer, uint8_t next_xfer, uint8_t stall_xfer);
static void umass_command_start(struct umass_softc *sc, uint8_t dir, void *data_ptr, uint32_t data_len, uint32_t data_timeout, umass_callback_t *callback, union ccb *ccb);
static uint8_t umass_bbb_get_max_lun(struct umass_softc *sc);
static void umass_cbi_start_status(struct umass_softc *sc);
static void umass_t_cbi_data_clear_stall_callback(struct usbd_xfer *xfer, uint8_t next_xfer, uint8_t stall_xfer);

static void umass_cam_quirk_cb(struct umass_softc *sc, union ccb *ccb, uint32_t residue, uint8_t status);
static uint8_t umass_scsi_transform(struct umass_softc *sc, uint8_t *cmd_ptr, uint8_t cmd_len);
static uint8_t umass_rbc_transform(struct umass_softc *sc, uint8_t *cmd_ptr, uint8_t cmd_len);
static uint8_t umass_ufi_transform(struct umass_softc *sc, uint8_t *cmd_ptr, uint8_t cmd_len);
static uint8_t umass_atapi_transform(struct umass_softc *sc, uint8_t *cmd_ptr, uint8_t cmd_len);
static uint8_t umass_no_transform(struct umass_softc *sc, uint8_t *cmd, uint8_t cmdlen);
static uint8_t umass_std_transform(struct umass_softc *sc, union ccb *ccb, uint8_t *cmd, uint8_t cmdlen);
static int umass_driver_loaded(struct module *mod, int what, void *arg);

#if USB_DEBUG
static void umass_bbb_dump_cbw(struct umass_softc *sc, umass_bbb_cbw_t *cbw);
static void umass_bbb_dump_csw(struct umass_softc *sc, umass_bbb_csw_t *csw);
static void umass_cbi_dump_cmd(struct umass_softc *sc, void *cmd, uint8_t cmdlen);
static void umass_dump_buffer(struct umass_softc *sc, uint8_t *buffer, uint32_t buflen, uint32_t printlen);

#endif

struct usbd_config umass_bbb_config[UMASS_T_BBB_MAX] = {

	[UMASS_T_BBB_RESET1] = {
		.type = UE_CONTROL,
		.endpoint = 0x00,	/* Control pipe */
		.direction = UE_DIR_ANY,
		.mh.bufsize = sizeof(struct usb_device_request),
		.mh.flags = {},
		.mh.callback = &umass_t_bbb_reset1_callback,
		.mh.timeout = 5000,	/* 5 seconds */
		.mh.interval = 500,	/* 500 milliseconds */
	},

	[UMASS_T_BBB_RESET2] = {
		.type = UE_CONTROL,
		.endpoint = 0x00,	/* Control pipe */
		.direction = UE_DIR_ANY,
		.mh.bufsize = sizeof(struct usb_device_request),
		.mh.flags = {},
		.mh.callback = &umass_t_bbb_reset2_callback,
		.mh.timeout = 5000,	/* 5 seconds */
		.mh.interval = 50,	/* 50 milliseconds */
	},

	[UMASS_T_BBB_RESET3] = {
		.type = UE_CONTROL,
		.endpoint = 0x00,	/* Control pipe */
		.direction = UE_DIR_ANY,
		.mh.bufsize = sizeof(struct usb_device_request),
		.mh.flags = {},
		.mh.callback = &umass_t_bbb_reset3_callback,
		.mh.timeout = 5000,	/* 5 seconds */
		.mh.interval = 50,	/* 50 milliseconds */
	},

	[UMASS_T_BBB_COMMAND] = {
		.type = UE_BULK,
		.endpoint = UE_ADDR_ANY,
		.direction = UE_DIR_OUT,
		.mh.bufsize = sizeof(umass_bbb_cbw_t),
		.mh.flags = {},
		.mh.callback = &umass_t_bbb_command_callback,
		.mh.timeout = 5000,	/* 5 seconds */
	},

	[UMASS_T_BBB_DATA_READ] = {
		.type = UE_BULK,
		.endpoint = UE_ADDR_ANY,
		.direction = UE_DIR_IN,
		.mh.bufsize = UMASS_BULK_SIZE,
		.mh.flags = {.proxy_buffer = 1,.short_xfer_ok = 1, UMASS_USB_FLAGS},
		.mh.callback = &umass_t_bbb_data_read_callback,
		.mh.timeout = 0,	/* overwritten later */
	},

	[UMASS_T_BBB_DATA_RD_CS] = {
		.type = UE_CONTROL,
		.endpoint = 0x00,	/* Control pipe */
		.direction = UE_DIR_ANY,
		.mh.bufsize = sizeof(struct usb_device_request),
		.mh.flags = {},
		.mh.callback = &umass_t_bbb_data_rd_cs_callback,
		.mh.timeout = 5000,	/* 5 seconds */
	},

	[UMASS_T_BBB_DATA_WRITE] = {
		.type = UE_BULK,
		.endpoint = UE_ADDR_ANY,
		.direction = UE_DIR_OUT,
		.mh.bufsize = UMASS_BULK_SIZE,
		.mh.flags = {.proxy_buffer = 1,.short_xfer_ok = 1, UMASS_USB_FLAGS},
		.mh.callback = &umass_t_bbb_data_write_callback,
		.mh.timeout = 0,	/* overwritten later */
	},

	[UMASS_T_BBB_DATA_WR_CS] = {
		.type = UE_CONTROL,
		.endpoint = 0x00,	/* Control pipe */
		.direction = UE_DIR_ANY,
		.mh.bufsize = sizeof(struct usb_device_request),
		.mh.flags = {},
		.mh.callback = &umass_t_bbb_data_wr_cs_callback,
		.mh.timeout = 5000,	/* 5 seconds */
	},

	[UMASS_T_BBB_STATUS] = {
		.type = UE_BULK,
		.endpoint = UE_ADDR_ANY,
		.direction = UE_DIR_IN,
		.mh.bufsize = sizeof(umass_bbb_csw_t),
		.mh.flags = {.short_xfer_ok = 1,},
		.mh.callback = &umass_t_bbb_status_callback,
		.mh.timeout = 5000,	/* ms */
	},
};

struct usbd_config umass_cbi_config[UMASS_T_CBI_MAX] = {

	[UMASS_T_CBI_RESET1] = {
		.type = UE_CONTROL,
		.endpoint = 0x00,	/* Control pipe */
		.direction = UE_DIR_ANY,
		.mh.bufsize = (sizeof(struct usb_device_request) +
		    UMASS_CBI_DIAGNOSTIC_CMDLEN),
		.mh.flags = {},
		.mh.callback = &umass_t_cbi_reset1_callback,
		.mh.timeout = 5000,	/* 5 seconds */
		.mh.interval = 500,	/* 500 milliseconds */
	},

	[UMASS_T_CBI_RESET2] = {
		.type = UE_CONTROL,
		.endpoint = 0x00,	/* Control pipe */
		.direction = UE_DIR_ANY,
		.mh.bufsize = sizeof(struct usb_device_request),
		.mh.flags = {},
		.mh.callback = &umass_t_cbi_reset2_callback,
		.mh.timeout = 5000,	/* 5 seconds */
		.mh.interval = 50,	/* 50 milliseconds */
	},

	[UMASS_T_CBI_RESET3] = {
		.type = UE_CONTROL,
		.endpoint = 0x00,	/* Control pipe */
		.direction = UE_DIR_ANY,
		.mh.bufsize = sizeof(struct usb_device_request),
		.mh.flags = {},
		.mh.callback = &umass_t_cbi_reset3_callback,
		.mh.timeout = 5000,	/* 5 seconds */
		.mh.interval = 50,	/* 50 milliseconds */
	},

	[UMASS_T_CBI_COMMAND] = {
		.type = UE_CONTROL,
		.endpoint = 0x00,	/* Control pipe */
		.direction = UE_DIR_ANY,
		.mh.bufsize = (sizeof(struct usb_device_request) +
		    UMASS_MAX_CMDLEN),
		.mh.flags = {},
		.mh.callback = &umass_t_cbi_command_callback,
		.mh.timeout = 5000,	/* 5 seconds */
	},

	[UMASS_T_CBI_DATA_READ] = {
		.type = UE_BULK,
		.endpoint = UE_ADDR_ANY,
		.direction = UE_DIR_IN,
		.mh.bufsize = UMASS_BULK_SIZE,
		.mh.flags = {.proxy_buffer = 1,.short_xfer_ok = 1, UMASS_USB_FLAGS},
		.mh.callback = &umass_t_cbi_data_read_callback,
		.mh.timeout = 0,	/* overwritten later */
	},

	[UMASS_T_CBI_DATA_RD_CS] = {
		.type = UE_CONTROL,
		.endpoint = 0x00,	/* Control pipe */
		.direction = UE_DIR_ANY,
		.mh.bufsize = sizeof(struct usb_device_request),
		.mh.flags = {},
		.mh.callback = &umass_t_cbi_data_rd_cs_callback,
		.mh.timeout = 5000,	/* 5 seconds */
	},

	[UMASS_T_CBI_DATA_WRITE] = {
		.type = UE_BULK,
		.endpoint = UE_ADDR_ANY,
		.direction = UE_DIR_OUT,
		.mh.bufsize = UMASS_BULK_SIZE,
		.mh.flags = {.proxy_buffer = 1,.short_xfer_ok = 1, UMASS_USB_FLAGS},
		.mh.callback = &umass_t_cbi_data_write_callback,
		.mh.timeout = 0,	/* overwritten later */
	},

	[UMASS_T_CBI_DATA_WR_CS] = {
		.type = UE_CONTROL,
		.endpoint = 0x00,	/* Control pipe */
		.direction = UE_DIR_ANY,
		.mh.bufsize = sizeof(struct usb_device_request),
		.mh.flags = {},
		.mh.callback = &umass_t_cbi_data_wr_cs_callback,
		.mh.timeout = 5000,	/* 5 seconds */
	},

	[UMASS_T_CBI_STATUS] = {
		.type = UE_INTERRUPT,
		.endpoint = UE_ADDR_ANY,
		.direction = UE_DIR_IN,
		.mh.flags = {.short_xfer_ok = 1,},
		.mh.bufsize = sizeof(umass_cbi_sbl_t),
		.mh.callback = &umass_t_cbi_status_callback,
		.mh.timeout = 5000,	/* ms */
	},

	[UMASS_T_CBI_RESET4] = {
		.type = UE_CONTROL,
		.endpoint = 0x00,	/* Control pipe */
		.direction = UE_DIR_ANY,
		.mh.bufsize = sizeof(struct usb_device_request),
		.mh.flags = {},
		.mh.callback = &umass_t_cbi_reset4_callback,
		.mh.timeout = 5000,	/* ms */
	},
};

/* If device cannot return valid inquiry data, fake it */
static const uint8_t fake_inq_data[SHORT_INQUIRY_LENGTH] = {
	0, /* removable */ 0x80, SCSI_REV_2, SCSI_REV_2,
	 /* additional_length */ 31, 0, 0, 0
};

#define	UFI_COMMAND_LENGTH	12	/* UFI commands are always 12 bytes */
#define	ATAPI_COMMAND_LENGTH	12	/* ATAPI commands are always 12 bytes */

static struct cam_sim *umass_sim[UMASS_MAXUNIT];
static struct mtx umass_mtx;

static devclass_t umass_devclass;

static device_method_t umass_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe, umass_probe),
	DEVMETHOD(device_attach, umass_attach),
	DEVMETHOD(device_detach, umass_detach),
	{0, 0}
};

static driver_t umass_driver = {
	.name = "umass",
	.methods = umass_methods,
	.size = sizeof(struct umass_softc),
};

DRIVER_MODULE(umass, ushub, umass_driver, umass_devclass, umass_driver_loaded, 0);
MODULE_DEPEND(umass, usb2_storage, 1, 1, 1);
MODULE_DEPEND(umass, usb2_core, 1, 1, 1);
MODULE_DEPEND(umass, cam, 1, 1, 1);

/*
 * USB device probe/attach/detach
 */

/*
 * Match the device we are seeing with the
 * devices supported.
 */
static struct umass_probe_proto
umass_probe_proto(device_t dev, struct usb_attach_arg *uaa)
{
	const struct umass_devdescr *udd = umass_devdescr;
	struct usb2_interface_descriptor *id;
	struct umass_probe_proto ret;

	bzero(&ret, sizeof(ret));

	/*
	 * An entry specifically for Y-E Data devices as they don't fit in
	 * the device description table.
	 */
	if ((uaa->info.idVendor == USB_VENDOR_YEDATA) &&
	    (uaa->info.idProduct == USB_PRODUCT_YEDATA_FLASHBUSTERU)) {

		/*
		 * Revisions < 1.28 do not handle the interrupt endpoint
		 * very well.
		 */
		if (uaa->info.bcdDevice < 0x128) {
			ret.proto = UMASS_PROTO_UFI | UMASS_PROTO_CBI;
		} else {
			ret.proto = UMASS_PROTO_UFI | UMASS_PROTO_CBI_I;
		}

		/*
		 * Revisions < 1.28 do not have the TEST UNIT READY command
		 * Revisions == 1.28 have a broken TEST UNIT READY
		 */
		if (uaa->info.bcdDevice <= 0x128) {
			ret.quirks |= NO_TEST_UNIT_READY;
		}
		ret.quirks |= RS_NO_CLEAR_UA | FLOPPY_SPEED;
		ret.error = 0;
		goto done;
	}
	/*
	 * Check the list of supported devices for a match. While looking,
	 * check for wildcarded and fully matched. First match wins.
	 */
	for (; udd->vid != VID_EOT; udd++) {
		if ((udd->vid == VID_WILDCARD) &&
		    (udd->pid == PID_WILDCARD) &&
		    (udd->rid == RID_WILDCARD)) {
			device_printf(dev, "ignoring invalid "
			    "wildcard quirk\n");
			continue;
		}
		if (((udd->vid == uaa->info.idVendor) ||
		    (udd->vid == VID_WILDCARD)) &&
		    ((udd->pid == uaa->info.idProduct) ||
		    (udd->pid == PID_WILDCARD))) {
			if (udd->rid == RID_WILDCARD) {
				ret.proto = udd->proto;
				ret.quirks = udd->quirks;
				ret.error = 0;
				goto done;
			} else if (udd->rid == uaa->info.bcdDevice) {
				ret.proto = udd->proto;
				ret.quirks = udd->quirks;
				ret.error = 0;
				goto done;
			}		/* else RID does not match */
		}
	}

	/* Check for a standards compliant device */
	id = usbd_get_interface_descriptor(uaa->iface);
	if ((id == NULL) ||
	    (id->bInterfaceClass != UICLASS_MASS)) {
		ret.error = ENXIO;
		goto done;
	}
	switch (id->bInterfaceSubClass) {
	case UISUBCLASS_SCSI:
		ret.proto |= UMASS_PROTO_SCSI;
		break;
	case UISUBCLASS_UFI:
		ret.proto |= UMASS_PROTO_UFI;
		break;
	case UISUBCLASS_RBC:
		ret.proto |= UMASS_PROTO_RBC;
		break;
	case UISUBCLASS_SFF8020I:
	case UISUBCLASS_SFF8070I:
		ret.proto |= UMASS_PROTO_ATAPI;
		break;
	default:
		device_printf(dev, "unsupported command "
		    "protocol %d\n", id->bInterfaceSubClass);
		ret.error = ENXIO;
		goto done;
	}

	switch (id->bInterfaceProtocol) {
	case UIPROTO_MASS_CBI:
		ret.proto |= UMASS_PROTO_CBI;
		break;
	case UIPROTO_MASS_CBI_I:
		ret.proto |= UMASS_PROTO_CBI_I;
		break;
	case UIPROTO_MASS_BBB_OLD:
	case UIPROTO_MASS_BBB:
		ret.proto |= UMASS_PROTO_BBB;
		break;
	default:
		device_printf(dev, "unsupported wire "
		    "protocol %d\n", id->bInterfaceProtocol);
		ret.error = ENXIO;
		goto done;
	}

	ret.error = 0;
done:
	return (ret);
}

int umass_probe(device_t dev)
{
	struct usb_attach_arg *uaa = device_get_ivars(dev);
	struct umass_probe_proto temp;

	if (uaa->usb2_mode != USB_MODE_HOST) {
		return (ENXIO);
	}
	//if (uaa->use_generic == 0) { //dpp
		/* give other drivers a try first */
	//	return (ENXIO);
	//}
	temp = umass_probe_proto(dev, uaa);

	return (temp.error);
}

int umass_attach(device_t dev)
{
	struct umass_softc *sc = device_get_softc(dev);
	struct usb_attach_arg *uaa = device_get_ivars(dev);
	struct umass_probe_proto temp = umass_probe_proto(dev, uaa);
	struct usbd_interface_descriptor *id;
	int32_t err;

	if (sc == NULL) {
		return (ENOMEM);
	}
	if (device_get_unit(dev) >= UMASS_MAXUNIT) {
		device_printf(dev, "Maxunit(%u) limit reached!\n",
		    UMASS_MAXUNIT);
		return (ENOMEM);
	}
	/*
	 * NOTE: the softc struct is bzero-ed in device_set_driver.
	 * We can safely call umass_detach without specifically
	 * initializing the struct.
	 */

	sc->sc_dev = dev;
	sc->sc_udev = uaa->device;
	sc->sc_proto = temp.proto;
	sc->sc_quirks = temp.quirks;
	sc->sc_unit = device_get_unit(dev);

	snprintf(sc->sc_name, sizeof(sc->sc_name),
	    "%s", device_get_nameunit(dev));

	device_set_usb2_desc(dev);

	/* get interface index */

	id = usbd_get_interface_descriptor(uaa->iface);
	if (id == NULL) {
		device_printf(dev, "failed to get "
		    "interface number\n");
		goto detach;
	}
	sc->sc_iface_no = id->bInterfaceNumber;

#if USB_DEBUG
	device_printf(dev, " ");

	switch (sc->sc_proto & UMASS_PROTO_COMMAND) {
	case UMASS_PROTO_SCSI:
		printf("SCSI");
		break;
	case UMASS_PROTO_ATAPI:
		printf("8070i (ATAPI)");
		break;
	case UMASS_PROTO_UFI:
		printf("UFI");
		break;
	case UMASS_PROTO_RBC:
		printf("RBC");
		break;
	default:
		printf("(unknown 0x%02x)",
		    sc->sc_proto & UMASS_PROTO_COMMAND);
		break;
	}

	printf(" over ");

	switch (sc->sc_proto & UMASS_PROTO_WIRE) {
	case UMASS_PROTO_BBB:
		printf("Bulk-Only");
		break;
	case UMASS_PROTO_CBI:		/* uses Comand/Bulk pipes */
		printf("CBI");
		break;
	case UMASS_PROTO_CBI_I:	/* uses Comand/Bulk/Interrupt pipes */
		printf("CBI with CCI");
		break;
	default:
		printf("(unknown 0x%02x)",
		    sc->sc_proto & UMASS_PROTO_WIRE);
	}

	printf("; quirks = 0x%04x\n", sc->sc_quirks);
#endif

	if (sc->sc_quirks & ALT_IFACE_1) {
		err = usbd_set_alt_interface_index
		    (uaa->device, uaa->info.bIfaceIndex, 1);

		if (err) {
			DPRINTF(sc, UDMASS_USB, "could not switch to "
			    "Alt Interface 1\n");
			goto detach;
		}
	}
	/* allocate all required USB transfers */

	if (sc->sc_proto & UMASS_PROTO_BBB) {

		err = usbd_transfer_setup(uaa->device,
		    &uaa->info.bIfaceIndex, sc->sc_xfer, umass_bbb_config,
		    UMASS_T_BBB_MAX, sc, &umass_mtx);

		/* skip reset first time */
		sc->sc_last_xfer_index = UMASS_T_BBB_COMMAND;

	} else if (sc->sc_proto & (UMASS_PROTO_CBI | UMASS_PROTO_CBI_I)) {

		err = usbd_transfer_setup(uaa->device,
		    &uaa->info.bIfaceIndex, sc->sc_xfer, umass_cbi_config,
		    (sc->sc_proto & UMASS_PROTO_CBI_I) ?
		    UMASS_T_CBI_MAX : (UMASS_T_CBI_MAX - 2), sc,
		    &umass_mtx);

		/* skip reset first time */
		sc->sc_last_xfer_index = UMASS_T_CBI_COMMAND;

	} else {
		err = USB_ERR_INVAL;
	}

	if (err) {
		device_printf(dev, "could not setup required "
		    "transfers, %s\n", usbd_errstr(err));
		goto detach;
	}
	sc->sc_transform =
	    (sc->sc_proto & UMASS_PROTO_SCSI) ? &umass_scsi_transform :
	    (sc->sc_proto & UMASS_PROTO_UFI) ? &umass_ufi_transform :
	    (sc->sc_proto & UMASS_PROTO_ATAPI) ? &umass_atapi_transform :
	    (sc->sc_proto & UMASS_PROTO_RBC) ? &umass_rbc_transform :
	    &umass_no_transform;

	/* from here onwards the device can be used. */

	if (sc->sc_quirks & SHUTTLE_INIT) {
		umass_init_shuttle(sc);
	}
	/* get the maximum LUN supported by the device */

	if (((sc->sc_proto & UMASS_PROTO_WIRE) == UMASS_PROTO_BBB) &&
	    !(sc->sc_quirks & NO_GETMAXLUN))
		sc->sc_maxlun = umass_bbb_get_max_lun(sc);
	else
		sc->sc_maxlun = 0;

	/* Prepare the SCSI command block */
	sc->cam_scsi_sense.opcode = SCSI_REQUEST_SENSE;
	sc->cam_scsi_test_unit_ready.opcode = SCSI_TEST_UNIT_READY;

	/*
	 * some devices need a delay after that the configuration value is
	 * set to function properly:
	 */
	usb2_pause_mtx(&Giant, USB_MS_HZ);

	/* register the SIM */
	err = umass_cam_attach_sim(sc);
	if (err) {
		goto detach;
	}
	/* scan the SIM */
	umass_cam_attach(sc);

	DPRINTF(sc, UDMASS_GEN, "Attach finished\n");

	return (0);			/* success */

detach:
	umass_detach(dev);
	return (ENXIO);			/* failure */
}


int umass_detach(device_t dev)
{
	struct umass_softc *sc = device_get_softc(dev);

	DPRINTF(sc, UDMASS_USB, "\n");

	/* teardown our statemachine */

	usbd_transfer_unsetup(sc->sc_xfer, UMASS_T_MAX);

#if (__FreeBSD_version >= 700037)
	mtx_lock(&umass_mtx);
#endif
	umass_cam_detach_sim(sc);

#if (__FreeBSD_version >= 700037)
	mtx_unlock(&umass_mtx);
#endif

	return (0);			/* success */
}


void umass_init_shuttle(struct umass_softc *sc)
{
	struct usb_device_request req;
	usbd_error_t err;
	uint8_t status[2] = {0, 0};

	/*
	 * The Linux driver does this, but no one can tell us what the
	 * command does.
	 */
	req.bmRequestType = UT_READ_VENDOR_DEVICE;
	req.bRequest = 1;		/* XXX unknown command */
	USETW(req.wValue, 0);
	req.wIndex[0] = sc->sc_iface_no;
	req.wIndex[1] = 0;
	USETW(req.wLength, sizeof(status));
	err = usbd_do_request(sc->sc_udev, &Giant, &req, &status);

	DPRINTF(sc, UDMASS_GEN, "Shuttle init returned 0x%02x%02x\n",
	    status[0], status[1]);
	return;
}

/*
 * Generic functions to handle transfers
 */

void umass_transfer_start(struct umass_softc *sc, uint8_t xfer_index)
{
	DPRINTF(sc, UDMASS_GEN, "transfer index = "
	    "%d\n", xfer_index);

	if (sc->sc_xfer[xfer_index]) {
		sc->sc_last_xfer_index = xfer_index;
		usbd_transfer_start(sc->sc_xfer[xfer_index]);
	} else {
		umass_cancel_ccb(sc);
	}
	return;
}

void umass_reset(struct umass_softc *sc)
{
	DPRINTF(sc, UDMASS_GEN, "resetting device\n");

	/*
	 * stop the last transfer, if not already stopped:
	 */
	usbd_transfer_stop(sc->sc_xfer[sc->sc_last_xfer_index]);
	umass_transfer_start(sc, 0);
	return;
}


int umass_driver_loaded(struct module *mod, int what, void *arg)
{
	uint16_t x;

	switch (what) {
	case MOD_LOAD:
		mtx_init(&umass_mtx, "UMASS lock", NULL, (MTX_DEF | MTX_RECURSE));
		break;

	case MOD_UNLOAD:
		for (x = 0; x != UMASS_MAXUNIT; x++) {
			/* cleanup */
			if (umass_sim[x])
				cam_sim_free(umass_sim[x], /* free_devq */ TRUE);
		}
		mtx_destroy(&umass_mtx);
		break;
	default:
		return (EOPNOTSUPP);
	}

	return (0);
}
