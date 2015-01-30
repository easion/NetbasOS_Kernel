//dpp:fixme
struct usbd_descriptor {
	uByte	bLength;
	uByte	bDescriptorType;
	uByte	bDescriptorSubtype;
} __packed;

struct usbd_interface_descriptor {
	uByte	bLength;
	uByte	bDescriptorType;
	uByte	bInterfaceNumber;
	uByte	bAlternateSetting;
	uByte	bNumEndpoints;
	uByte	bInterfaceClass;
	uByte	bInterfaceSubClass;
	uByte	bInterfaceProtocol;
	uByte	iInterface;
} __packed;


struct usbd_endpoint_descriptor {
	uByte	bLength;
	uByte	bDescriptorType;
	uByte	bEndpointAddress;
#define	UE_GET_DIR(a)	((a) & 0x80)
#define	UE_SET_DIR(a,d)	((a) | (((d)&1) << 7))
#define	UE_DIR_IN	0x80
#define	UE_DIR_OUT	0x00
#define	UE_DIR_ANY	0xff		/* for internal use only! */
#define	UE_ADDR		0x0f
#define	UE_ADDR_ANY	0xff		/* for internal use only! */
#define	UE_GET_ADDR(a)	((a) & UE_ADDR)
	uByte	bmAttributes;
#define	UE_XFERTYPE	0x03
#define	UE_CONTROL	0x00
#define	UE_ISOCHRONOUS	0x01
#define	UE_BULK	0x02
#define	UE_INTERRUPT	0x03
#define	UE_BULK_INTR	0xfe		/* for internal use only! */
#define	UE_TYPE_ANY	0xff		/* for internal use only! */
#define	UE_GET_XFERTYPE(a)	((a) & UE_XFERTYPE)
#define	UE_ISO_TYPE	0x0c
#define	UE_ISO_ASYNC	0x04
#define	UE_ISO_ADAPT	0x08
#define	UE_ISO_SYNC	0x0c
#define	UE_GET_ISO_TYPE(a)	((a) & UE_ISO_TYPE)
	uWord	wMaxPacketSize;
#define	UE_ZERO_MPS 0xFFFF		/* for internal use only */
	uByte	bInterval;
} __packed;

struct usbd_string_descriptor {
	uByte	bLength;
	uByte	bDescriptorType;
	uWord	bString[126];
	uByte	bUnused;
} __packed;

#define UICLASS_MASS		0x08


#define USB_ERR_INVAL -1
#define USB_MS_HZ 10
#define CAM_MAX_CDBLEN 255 //dpp
#define SHORT_INQUIRY_LENGTH 36

#define SID_ANSI_REV(inq_data) ((inq_data)->version & 0x07)
#define         SCSI_REV_0              0
#define         SCSI_REV_CCS            1
#define         SCSI_REV_2              2
#define         SCSI_REV_SPC            3
#define         SCSI_REV_SPC2           4



struct scsi_sense
{
        u_int8_t opcode;
        u_int8_t byte2;
        u_int8_t unused[2];
        u_int8_t length;
        u_int8_t control;
};

struct scsi_inquiry
{
        u_int8_t opcode;
        u_int8_t byte2;
#define SI_EVPD 0x01
        u_int8_t page_code;
        u_int8_t reserved;
        u_int8_t length;
        u_int8_t control;
};


struct scsi_test_unit_ready
{
        u_int8_t opcode;
        u_int8_t byte2;
        u_int8_t unused[3];
        u_int8_t control;
};

