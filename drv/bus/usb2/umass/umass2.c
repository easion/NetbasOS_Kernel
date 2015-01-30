
#include "../netbas.h"
#include <usb_port.h>
#include <usb.h>
#include <usb_subr.h>

#include "umass.h"


/* Also already merged from NetBSD:
 *	$NetBSD: umass.c,v 1.67 2001/11/25 19:05:22 augustss Exp $
 *	$NetBSD: umass.c,v 1.90 2002/11/04 19:17:33 pooka Exp $
 *	$NetBSD: umass.c,v 1.108 2003/11/07 17:03:25 wiz Exp $
 *	$NetBSD: umass.c,v 1.109 2003/12/04 13:57:31 keihan Exp $
 */

/*
 * Universal Serial Bus Mass Storage Class specs:
 * http://www.usb.org/developers/devclass_docs/usb_msc_overview_1.2.pdf
 * http://www.usb.org/developers/devclass_docs/usbmassbulk_10.pdf
 * http://www.usb.org/developers/devclass_docs/usb_msc_cbi_1.1.pdf
 * http://www.usb.org/developers/devclass_docs/usbmass-ufi10.pdf
 */

/*
 * Ported to NetBSD by Lennart Augustsson <augustss@NetBSD.org>.
 * Parts of the code written by Jason R. Thorpe <thorpej@shagadelic.org>.
 */

/*
 * The driver handles 3 Wire Protocols
 * - Command/Bulk/Interrupt (CBI)
 * - Command/Bulk/Interrupt with Command Completion Interrupt (CBI with CCI)
 * - Mass Storage Bulk-Only (BBB)
 *   (BBB refers Bulk/Bulk/Bulk for Command/Data/Status phases)
 *
 * Over these wire protocols it handles the following command protocols
 * - SCSI
 * - UFI (floppy command set)
 * - 8070i (ATAPI)
 *
 * UFI and 8070i (ATAPI) are transformed versions of the SCSI command set. The
 * sc->sc_transform method is used to convert the commands into the appropriate
 * format (if at all necessary). For example, UFI requires all commands to be
 * 12 bytes in length amongst other things.
 *
 * The source code below is marked and can be split into a number of pieces
 * (in this order):
 *
 * - probe/attach/detach
 * - generic transfer routines
 * - BBB
 * - CBI
 * - CBI_I (in addition to functions from CBI)
 * - CAM (Common Access Method)
 * - SCSI
 * - UFI
 * - 8070i (ATAPI)
 *
 * The protocols are implemented using a state machine, for the transfers as
 * well as for the resets. The state machine is contained in umass_t_*_callback.
 * The state machine is started through either umass_command_start() or
 * umass_reset().
 *
 * The reason for doing this is a) CAM performs a lot better this way and b) it
 * avoids using tsleep from interrupt context (for example after a failed
 * transfer).
 */

/*
 * The SCSI related part of this driver has been derived from the
 * dev/ppbus/vpo.c driver, by Nicolas Souchu (nsouch@freebsd.org).
 *
 * The CAM layer uses so called actions which are messages sent to the host
 * adapter for completion. The actions come in through umass_cam_action. The
 * appropriate block of routines is called depending on the transport protocol
 * in use. When the transfer has finished, these routines call
 * umass_cam_cb again to complete the CAM command.
 */


void umass_cancel_ccb(struct umass_softc *sc)
{
	union ccb *ccb;

	mtx_assert(&umass_mtx, MA_OWNED);

	ccb = sc->sc_transfer.ccb;
	sc->sc_transfer.ccb = NULL;
	sc->sc_last_xfer_index = 0;

	if (ccb) {
		(sc->sc_transfer.callback)
		    (sc, ccb, (sc->sc_transfer.data_len -
		    sc->sc_transfer.actlen), STATUS_WIRE_FAILED);
	}
	return;
}

void umass_tr_error(struct usbd_xfer *xfer)
{
	struct umass_softc *sc = xfer->priv_sc;

	if (xfer->error != USB_ERR_CANCELLED) {

		DPRINTF(sc, UDMASS_GEN, "transfer error, %s -> "
		    "reset\n", usb2_errstr(xfer->error));
	}
	umass_cancel_ccb(sc);
	return;
}


void umass_command_start(struct umass_softc *sc, uint8_t dir,
    void *data_ptr, uint32_t data_len,
    uint32_t data_timeout, umass_callback_t *callback,
    union ccb *ccb)
{
	sc->sc_transfer.lun = ccb->ccb_h.target_lun;

	/*
	 * NOTE: assumes that "sc->sc_transfer.cmd_data" and
	 * "sc->sc_transfer.cmd_len" has been properly
	 * initialized.
	 */

	sc->sc_transfer.dir = data_len ? dir : DIR_NONE;
	sc->sc_transfer.data_ptr = data_ptr;
	sc->sc_transfer.data_len = data_len;
	sc->sc_transfer.data_rem = data_len;
	sc->sc_transfer.data_timeout = (data_timeout + UMASS_TIMEOUT);

	sc->sc_transfer.actlen = 0;
	sc->sc_transfer.callback = callback;
	sc->sc_transfer.ccb = ccb;

	if (sc->sc_xfer[sc->sc_last_xfer_index]) {
		usb2_transfer_start(sc->sc_xfer[sc->sc_last_xfer_index]);
	} else {
		ccb->ccb_h.status = CAM_TID_INVALID;
		xpt_done(ccb);
	}
	return;
}

uint8_t umass_bbb_get_max_lun(struct umass_softc *sc)
{
	struct usb_device_request req;
	usb2_error_t err;
	uint8_t buf = 0;

	/* The Get Max Lun command is a class-specific request. */
	req.bmRequestType = UT_READ_CLASS_INTERFACE;
	req.bRequest = UR_BBB_GET_MAX_LUN;
	USETW(req.wValue, 0);
	req.wIndex[0] = sc->sc_iface_no;
	req.wIndex[1] = 0;
	USETW(req.wLength, 1);

	err = usb2_do_request(sc->sc_udev, &Giant, &req, &buf);
	if (err) {
		buf = 0;

		/* Device doesn't support Get Max Lun request. */
		printf("%s: Get Max Lun not supported (%s)\n",
		    sc->sc_name, usb2_errstr(err));
	}
	return (buf);
}



uint8_t umass_rbc_transform(struct umass_softc *sc, uint8_t *cmd_ptr, uint8_t cmd_len)
{
	if ((cmd_len == 0) ||
	    (cmd_len > sizeof(sc->sc_transfer.cmd_data))) {
		DPRINTF(sc, UDMASS_SCSI, "Invalid command "
		    "length: %d bytes\n", cmd_len);
		return (0);		/* failure */
	}
	switch (cmd_ptr[0]) {
		/* these commands are defined in RBC: */
	case SCSI_READ_10:
	case READ_CAPACITY:
	case START_STOP_UNIT:
	case SYNCHRONIZE_CACHE:
	case SCSI_WRITE_10:
	case 0x2f:			/* VERIFY_10 is absent from
					 * scsi_all.h??? */
	case INQUIRY:
	case MODE_SELECT_10:
	case MODE_SENSE_10:
	case SCSI_TEST_UNIT_READY:
	case WRITE_BUFFER:
		/*
		 * The following commands are not listed in my copy of the
		 * RBC specs. CAM however seems to want those, and at least
		 * the Sony DSC device appears to support those as well
		 */
	case SCSI_REQUEST_SENSE:
	case PREVENT_ALLOW:

		bcopy(cmd_ptr, sc->sc_transfer.cmd_data, cmd_len);

		if ((sc->sc_quirks & RBC_PAD_TO_12) && (cmd_len < 12)) {
			bzero(sc->sc_transfer.cmd_data + cmd_len, 12 - cmd_len);
			cmd_len = 12;
		}
		sc->sc_transfer.cmd_len = cmd_len;
		return (1);		/* sucess */

		/* All other commands are not legal in RBC */
	default:
		DPRINTF(sc, UDMASS_SCSI, "Unsupported RBC "
		    "command 0x%02x\n", cmd_ptr[0]);
		return (0);		/* failure */
	}
}

uint8_t umass_ufi_transform(struct umass_softc *sc, uint8_t *cmd_ptr,
    uint8_t cmd_len)
{
	if ((cmd_len == 0) ||
	    (cmd_len > sizeof(sc->sc_transfer.cmd_data))) {
		DPRINTF(sc, UDMASS_SCSI, "Invalid command "
		    "length: %d bytes\n", cmd_len);
		return (0);		/* failure */
	}
	/* An UFI command is always 12 bytes in length */
	sc->sc_transfer.cmd_len = UFI_COMMAND_LENGTH;

	/* Zero the command data */
	bzero(sc->sc_transfer.cmd_data, UFI_COMMAND_LENGTH);

	switch (cmd_ptr[0]) {
		/*
		 * Commands of which the format has been verified. They
		 * should work. Copy the command into the (zeroed out)
		 * destination buffer.
		 */
	case SCSI_TEST_UNIT_READY:
		if (sc->sc_quirks & NO_TEST_UNIT_READY) {
			/*
			 * Some devices do not support this command. Start
			 * Stop Unit should give the same results
			 */
			DPRINTF(sc, UDMASS_UFI, "Converted SCSI_TEST_UNIT_READY "
			    "to START_UNIT\n");

			sc->sc_transfer.cmd_data[0] = START_STOP_UNIT;
			sc->sc_transfer.cmd_data[4] = SSS_START;
			return (1);
		}
		break;

	case REZERO_UNIT:
	case SCSI_REQUEST_SENSE:
	case FORMAT_UNIT:
	case INQUIRY:
	case START_STOP_UNIT:
	case SEND_DIAGNOSTIC:
	case PREVENT_ALLOW:
	case READ_CAPACITY:
	case SCSI_READ_10:
	case SCSI_WRITE_10:
	case POSITION_TO_ELEMENT:	/* SEEK_10 */
	case WRITE_AND_VERIFY:
	case VERIFY:
	case MODE_SELECT_10:
	case MODE_SENSE_10:
	case READ_12:
	case WRITE_12:
	case READ_FORMAT_CAPACITIES:
		break;

		/*
		 * SYNCHRONIZE_CACHE isn't supported by UFI, nor should it be
		 * required for UFI devices, so it is appropriate to fake
		 * success.
		 */
	case SYNCHRONIZE_CACHE:
		return (2);

	default:
		DPRINTF(sc, UDMASS_SCSI, "Unsupported UFI "
		    "command 0x%02x\n", cmd_ptr[0]);
		return (0);		/* failure */
	}

	bcopy(cmd_ptr, sc->sc_transfer.cmd_data, cmd_len);
	return (1);			/* success */
}

/*
 * 8070i (ATAPI) specific functions
 */
uint8_t umass_atapi_transform(struct umass_softc *sc, uint8_t *cmd_ptr,
    uint8_t cmd_len)
{
	if ((cmd_len == 0) ||
	    (cmd_len > sizeof(sc->sc_transfer.cmd_data))) {
		DPRINTF(sc, UDMASS_SCSI, "Invalid command "
		    "length: %d bytes\n", cmd_len);
		return (0);		/* failure */
	}
	/* An ATAPI command is always 12 bytes in length. */
	sc->sc_transfer.cmd_len = ATAPI_COMMAND_LENGTH;

	/* Zero the command data */
	bzero(sc->sc_transfer.cmd_data, ATAPI_COMMAND_LENGTH);

	switch (cmd_ptr[0]) {
		/*
		 * Commands of which the format has been verified. They
		 * should work. Copy the command into the destination
		 * buffer.
		 */
	case INQUIRY:
		/*
		 * some drives wedge when asked for full inquiry
		 * information.
		 */
		if (sc->sc_quirks & FORCE_SHORT_INQUIRY) {
			bcopy(cmd_ptr, sc->sc_transfer.cmd_data, cmd_len);

			sc->sc_transfer.cmd_data[4] = SHORT_INQUIRY_LENGTH;
			return (1);
		}
		break;

	case SCSI_TEST_UNIT_READY:
		if (sc->sc_quirks & NO_TEST_UNIT_READY) {
			DPRINTF(sc, UDMASS_SCSI, "Converted SCSI_TEST_UNIT_READY "
			    "to START_UNIT\n");
			sc->sc_transfer.cmd_data[0] = START_STOP_UNIT;
			sc->sc_transfer.cmd_data[4] = SSS_START;
			return (1);
		}
		break;

	case REZERO_UNIT:
	case SCSI_REQUEST_SENSE:
	case START_STOP_UNIT:
	case SEND_DIAGNOSTIC:
	case PREVENT_ALLOW:
	case READ_CAPACITY:
	case SCSI_READ_10:
	case SCSI_WRITE_10:
	case POSITION_TO_ELEMENT:	/* SEEK_10 */
	case SYNCHRONIZE_CACHE:
	case MODE_SELECT_10:
	case MODE_SENSE_10:
	case READ_BUFFER:
	case 0x42:			/* READ_SUBCHANNEL */
	case 0x43:			/* READ_TOC */
	case 0x44:			/* READ_HEADER */
	case 0x47:			/* PLAY_MSF (Play Minute/Second/Frame) */
	case 0x48:			/* PLAY_TRACK */
	case 0x49:			/* PLAY_TRACK_REL */
	case 0x4b:			/* PAUSE */
	case 0x51:			/* READ_DISK_INFO */
	case 0x52:			/* READ_TRACK_INFO */
	case 0x54:			/* SEND_OPC */
	case 0x59:			/* READ_MASTER_CUE */
	case 0x5b:			/* CLOSE_TR_SESSION */
	case 0x5c:			/* READ_BUFFER_CAP */
	case 0x5d:			/* SEND_CUE_SHEET */
	case 0xa1:			/* BLANK */
	case 0xa5:			/* PLAY_12 */
	case 0xa6:			/* EXCHANGE_MEDIUM */
	case 0xad:			/* READ_DVD_STRUCTURE */
	case 0xbb:			/* SET_CD_SPEED */
	case 0xe5:			/* READ_TRACK_INFO_PHILIPS */
		break;;

	case READ_12:
	case WRITE_12:
	default:
		DPRINTF(sc, UDMASS_SCSI, "Unsupported ATAPI "
		    "command 0x%02x - trying anyway\n",
		    cmd_ptr[0]);
		break;;
	}

	bcopy(cmd_ptr, sc->sc_transfer.cmd_data, cmd_len);
	return (1);			/* success */
}

uint8_t umass_no_transform(struct umass_softc *sc, uint8_t *cmd,
    uint8_t cmdlen)
{
	return (0);			/* failure */
}

uint8_t umass_std_transform(struct umass_softc *sc, union ccb *ccb,
    uint8_t *cmd, uint8_t cmdlen)
{
	uint8_t retval;

	retval = (sc->sc_transform) (sc, cmd, cmdlen);

	if (retval == 2) {
		ccb->ccb_h.status = CAM_REQ_CMP;
		xpt_done(ccb);
		return (0);
	} else if (retval == 0) {
		ccb->ccb_h.status = CAM_REQ_INVALID;
		xpt_done(ccb);
		return (0);
	}
	/* Command should be executed */
	return (1);
}

#if USB_DEBUG
void umass_bbb_dump_cbw(struct umass_softc *sc, umass_bbb_cbw_t *cbw)
{
	uint8_t *c = cbw->CBWCDB;

	uint32_t dlen = UGETDW(cbw->dCBWDataTransferLength);
	uint32_t tag = UGETDW(cbw->dCBWTag);

	uint8_t clen = cbw->bCDBLength;
	uint8_t flags = cbw->bCBWFlags;
	uint8_t lun = cbw->bCBWLUN;

	DPRINTF(sc, UDMASS_BBB, "CBW %d: cmd = %db "
	    "(0x%02x%02x%02x%02x%02x%02x%s), "
	    "data = %db, lun = %d, dir = %s\n",
	    tag, clen,
	    c[0], c[1], c[2], c[3], c[4], c[5], (clen > 6 ? "..." : ""),
	    dlen, lun, (flags == CBWFLAGS_IN ? "in" :
	    (flags == CBWFLAGS_OUT ? "out" : "<invalid>")));
	return;
}

void umass_bbb_dump_csw(struct umass_softc *sc, umass_bbb_csw_t *csw)
{
	uint32_t sig = UGETDW(csw->dCSWSignature);
	uint32_t tag = UGETDW(csw->dCSWTag);
	uint32_t res = UGETDW(csw->dCSWDataResidue);
	uint8_t status = csw->bCSWStatus;

	DPRINTF(sc, UDMASS_BBB, "CSW %d: sig = 0x%08x (%s), tag = 0x%08x, "
	    "res = %d, status = 0x%02x (%s)\n",
	    tag, sig, (sig == CSWSIGNATURE ? "valid" : "invalid"),
	    tag, res,
	    status, (status == CSWSTATUS_GOOD ? "good" :
	    (status == CSWSTATUS_FAILED ? "failed" :
	    (status == CSWSTATUS_PHASE ? "phase" : "<invalid>"))));
	return;
}

void umass_cbi_dump_cmd(struct umass_softc *sc, void *cmd, uint8_t cmdlen)
{
	uint8_t *c = cmd;
	uint8_t dir = sc->sc_transfer.dir;

	DPRINTF(sc, UDMASS_BBB, "cmd = %db "
	    "(0x%02x%02x%02x%02x%02x%02x%s), "
	    "data = %db, dir = %s\n",
	    cmdlen,
	    c[0], c[1], c[2], c[3], c[4], c[5], (cmdlen > 6 ? "..." : ""),
	    sc->sc_transfer.data_len,
	    (dir == DIR_IN ? "in" :
	    (dir == DIR_OUT ? "out" :
	    (dir == DIR_NONE ? "no data phase" : "<invalid>"))));
	return;
}

void umass_dump_buffer(struct umass_softc *sc, uint8_t *buffer, uint32_t buflen,
    uint32_t printlen)
{
	uint32_t i, j;
	char s1[40];
	char s2[40];
	char s3[5];

	s1[0] = '\0';
	s3[0] = '\0';

	sprintf(s2, " buffer=%p, buflen=%d", buffer, buflen);
	for (i = 0; (i < buflen) && (i < printlen); i++) {
		j = i % 16;
		if (j == 0 && i != 0) {
			DPRINTF(sc, UDMASS_GEN, "0x %s%s\n",
			    s1, s2);
			s2[0] = '\0';
		}
		sprintf(&s1[j * 2], "%02x", buffer[i] & 0xff);
	}
	if (buflen > printlen)
		sprintf(s3, " ...");
	DPRINTF(sc, UDMASS_GEN, "0x %s%s%s\n",
	    s1, s2, s3);
	return;
}

#endif


/*
 * SCSI specific functions
 */

uint8_t umass_scsi_transform(struct umass_softc *sc, uint8_t *cmd_ptr,
    uint8_t cmd_len)
{
	if ((cmd_len == 0) ||
	    (cmd_len > sizeof(sc->sc_transfer.cmd_data))) {
		DPRINTF(sc, UDMASS_SCSI, "Invalid command "
		    "length: %d bytes\n", cmd_len);
		return (0);		/* failure */
	}
	sc->sc_transfer.cmd_len = cmd_len;

	switch (cmd_ptr[0]) {
	case SCSI_TEST_UNIT_READY:
		if (sc->sc_quirks & NO_TEST_UNIT_READY) {
			DPRINTF(sc, UDMASS_SCSI, "Converted SCSI_TEST_UNIT_READY "
			    "to START_UNIT\n");
			bzero(sc->sc_transfer.cmd_data, cmd_len);
			sc->sc_transfer.cmd_data[0] = START_STOP_UNIT;
			sc->sc_transfer.cmd_data[4] = SSS_START;
			return (1);
		}
		break;

	case INQUIRY:
		/*
		 * some drives wedge when asked for full inquiry
		 * information.
		 */
		if (sc->sc_quirks & FORCE_SHORT_INQUIRY) {
			bcopy(cmd_ptr, sc->sc_transfer.cmd_data, cmd_len);
			sc->sc_transfer.cmd_data[4] = SHORT_INQUIRY_LENGTH;
			return (1);
		}
		break;
	}

	bcopy(cmd_ptr, sc->sc_transfer.cmd_data, cmd_len);
	return (1);
}

struct cam_devq *
cam_simq_alloc(u_int32_t max_sim_transactions)
{
        //return (cam_devq_alloc(/*size*/0, max_sim_transactions));
}

void
cam_simq_free(struct cam_devq *devq)
{
       // cam_devq_free(devq);
}

struct cam_sim *
cam_sim_alloc(sim_action_func sim_action, sim_poll_func sim_poll,
              const char *sim_name, void *softc, u_int32_t unit,
              int max_dev_transactions,
              int max_tagged_dev_transactions, struct cam_devq *queue)
{
#if 0
        struct cam_sim *sim;

        /*
         * If this is the xpt layer creating a sim, then it's OK
         * to wait for an allocation.
         *
         * XXX Should we pass in a flag to indicate that wait is OK?
         */
        if (strcmp(sim_name, "xpt") == 0)
                sim = (struct cam_sim *)malloc(sizeof(struct cam_sim),
                                               M_DEVBUF, M_WAITOK);
        else
                sim = (struct cam_sim *)malloc(sizeof(struct cam_sim),
                                               M_DEVBUF, M_NOWAIT);

        if (sim != NULL) {
                sim->sim_action = sim_action;
                sim->sim_poll = sim_poll;
                sim->sim_name = sim_name;
                sim->softc = softc;
                sim->path_id = CAM_PATH_ANY;
                sim->unit_number = unit;
                sim->bus_id = 0;        /* set in xpt_bus_register */
                sim->max_tagged_dev_openings = max_tagged_dev_transactions;
                sim->max_dev_openings = max_dev_transactions;
                sim->flags = 0;
                callout_handle_init(&sim->c_handle);
                sim->devq = queue;
        }

        return (sim);
#endif
}

void
cam_sim_free(struct cam_sim *sim, int free_devq)
{
        //if (free_devq)
         //       cam_simq_free(sim->devq);
        free(sim, M_DEVBUF);
}

void
cam_sim_set_path(struct cam_sim *sim, u_int32_t path_id)
{
       // sim->path_id = path_id;
}
