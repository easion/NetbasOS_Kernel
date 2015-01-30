
#include "../netbas.h"
#include <usb_port.h>
#include <usb.h>
#include <usb_subr.h>
#include "umass.h"


/*
 * BBB protocol specific functions
 */

void umass_t_bbb_reset1_callback(struct usbd_xfer *xfer)
{
	struct umass_softc *sc = xfer->priv_sc;
	struct usb_device_request req;

	switch (USB_GET_STATE(xfer)) {
	case USB_ST_TRANSFERRED:
		umass_transfer_start(sc, UMASS_T_BBB_RESET2);
		return;

	case USB_ST_SETUP:
		/*
		 * Reset recovery (5.3.4 in Universal Serial Bus Mass Storage Class)
		 *
		 * For Reset Recovery the host shall issue in the following order:
		 * a) a Bulk-Only Mass Storage Reset
		 * b) a Clear Feature HALT to the Bulk-In endpoint
		 * c) a Clear Feature HALT to the Bulk-Out endpoint
		 *
		 * This is done in 3 steps, using 3 transfers:
		 * UMASS_T_BBB_RESET1
		 * UMASS_T_BBB_RESET2
		 * UMASS_T_BBB_RESET3
		 */

		DPRINTF(sc, UDMASS_BBB, "BBB reset!\n");

		req.bmRequestType = UT_WRITE_CLASS_INTERFACE;
		req.bRequest = UR_BBB_RESET;	/* bulk only reset */
		USETW(req.wValue, 0);
		req.wIndex[0] = sc->sc_iface_no;
		req.wIndex[1] = 0;
		USETW(req.wLength, 0);

		usb2_copy_in(xfer->frbuffers, 0, &req, sizeof(req));

		xfer->frlengths[0] = sizeof(req);
		xfer->nframes = 1;
		usb2_start_hardware(xfer);
		return;

	default:			/* Error */
		umass_tr_error(xfer);
		return;

	}
}

void umass_t_bbb_reset2_callback(struct usbd_xfer *xfer)
{
	umass_t_bbb_data_clear_stall_callback(xfer, UMASS_T_BBB_RESET3,
	    UMASS_T_BBB_DATA_READ);
	return;
}

void umass_t_bbb_reset3_callback(struct usbd_xfer *xfer)
{
	umass_t_bbb_data_clear_stall_callback(xfer, UMASS_T_BBB_COMMAND,
	    UMASS_T_BBB_DATA_WRITE);
	return;
}

void umass_t_bbb_data_clear_stall_callback(struct usbd_xfer *xfer,
    uint8_t next_xfer,
    uint8_t stall_xfer)
{
	struct umass_softc *sc = xfer->priv_sc;

	switch (USB_GET_STATE(xfer)) {
	case USB_ST_TRANSFERRED:
tr_transferred:
		umass_transfer_start(sc, next_xfer);
		return;

	case USB_ST_SETUP:
		if (usb2_clear_stall_callback(xfer, sc->sc_xfer[stall_xfer])) {
			goto tr_transferred;
		}
		return;

	default:			/* Error */
		umass_tr_error(xfer);
		return;

	}
}

void umass_t_bbb_command_callback(struct usbd_xfer *xfer)
{
	struct umass_softc *sc = xfer->priv_sc;
	union ccb *ccb = sc->sc_transfer.ccb;
	uint32_t tag;

	switch (USB_GET_STATE(xfer)) {
	case USB_ST_TRANSFERRED:
		umass_transfer_start
		    (sc, ((sc->sc_transfer.dir == DIR_IN) ? UMASS_T_BBB_DATA_READ :
		    (sc->sc_transfer.dir == DIR_OUT) ? UMASS_T_BBB_DATA_WRITE :
		    UMASS_T_BBB_STATUS));
		return;

	case USB_ST_SETUP:

		sc->sc_status_try = 0;

		if (ccb) {

			/*
		         * the initial value is not important,
		         * as long as the values are unique:
		         */
			tag = UGETDW(sc->cbw.dCBWTag) + 1;

			USETDW(sc->cbw.dCBWSignature, CBWSIGNATURE);
			USETDW(sc->cbw.dCBWTag, tag);

			/*
		         * dCBWDataTransferLength:
		         *   This field indicates the number of bytes of data that the host
		         *   intends to transfer on the IN or OUT Bulk endpoint(as indicated by
		         *   the Direction bit) during the execution of this command. If this
		         *   field is set to 0, the device will expect that no data will be
		         *   transferred IN or OUT during this command, regardless of the value
		         *   of the Direction bit defined in dCBWFlags.
		         */
			USETDW(sc->cbw.dCBWDataTransferLength, sc->sc_transfer.data_len);

			/*
		         * dCBWFlags:
		         *   The bits of the Flags field are defined as follows:
		         *     Bits 0-6  reserved
		         *     Bit  7    Direction - this bit shall be ignored if the
		         *                           dCBWDataTransferLength field is zero.
		         *               0 = data Out from host to device
		         *               1 = data In from device to host
		         */
			sc->cbw.bCBWFlags = ((sc->sc_transfer.dir == DIR_IN) ?
			    CBWFLAGS_IN : CBWFLAGS_OUT);
			sc->cbw.bCBWLUN = sc->sc_transfer.lun;

			if (sc->sc_transfer.cmd_len > sizeof(sc->cbw.CBWCDB)) {
				sc->sc_transfer.cmd_len = sizeof(sc->cbw.CBWCDB);
				DPRINTF(sc, UDMASS_BBB, "Truncating long command!\n");
			}
			sc->cbw.bCDBLength = sc->sc_transfer.cmd_len;

			bcopy(sc->sc_transfer.cmd_data, sc->cbw.CBWCDB,
			    sc->sc_transfer.cmd_len);

			bzero(sc->sc_transfer.cmd_data + sc->sc_transfer.cmd_len,
			    sizeof(sc->cbw.CBWCDB) - sc->sc_transfer.cmd_len);

			DIF(UDMASS_BBB, umass_bbb_dump_cbw(sc, &sc->cbw));

			usb2_copy_in(xfer->frbuffers, 0, &sc->cbw, sizeof(sc->cbw));

			xfer->frlengths[0] = sizeof(sc->cbw);
			usb2_start_hardware(xfer);
		}
		return;

	default:			/* Error */
		umass_tr_error(xfer);
		return;

	}
}

void umass_t_bbb_data_read_callback(struct usbd_xfer *xfer)
{
	struct umass_softc *sc = xfer->priv_sc;
	uint32_t max_bulk = xfer->max_data_length;

	switch (USB_GET_STATE(xfer)) {
	case USB_ST_TRANSFERRED:
		if (!xfer->flags2.ext_buffer) {
			usb2_copy_out(xfer->frbuffers, 0,
			    sc->sc_transfer.data_ptr, xfer->actlen);
		}
		sc->sc_transfer.data_rem -= xfer->actlen;
		sc->sc_transfer.data_ptr += xfer->actlen;
		sc->sc_transfer.actlen += xfer->actlen;

		if (xfer->actlen < xfer->sumlen) {
			/* short transfer */
			sc->sc_transfer.data_rem = 0;
		}
	case USB_ST_SETUP:
		DPRINTF(sc, UDMASS_BBB, "max_bulk=%d, data_rem=%d\n",
		    max_bulk, sc->sc_transfer.data_rem);

		if (sc->sc_transfer.data_rem == 0) {
			umass_transfer_start(sc, UMASS_T_BBB_STATUS);
			return;
		}
		if (max_bulk > sc->sc_transfer.data_rem) {
			max_bulk = sc->sc_transfer.data_rem;
		}
		xfer->timeout = sc->sc_transfer.data_timeout;
		xfer->frlengths[0] = max_bulk;

		if (xfer->flags2.ext_buffer) {
			usb2_set_frame_data(xfer, sc->sc_transfer.data_ptr, 0);
		}
		usb2_start_hardware(xfer);
		return;

	default:			/* Error */
		if (xfer->error == USB_ERR_CANCELLED) {
			umass_tr_error(xfer);
		} else {
			umass_transfer_start(sc, UMASS_T_BBB_DATA_RD_CS);
		}
		return;

	}
}

void umass_t_bbb_data_rd_cs_callback(struct usbd_xfer *xfer)
{
	umass_t_bbb_data_clear_stall_callback(xfer, UMASS_T_BBB_STATUS,
	    UMASS_T_BBB_DATA_READ);
	return;
}

void umass_t_bbb_data_write_callback(struct usbd_xfer *xfer)
{
	struct umass_softc *sc = xfer->priv_sc;
	uint32_t max_bulk = xfer->max_data_length;

	switch (USB_GET_STATE(xfer)) {
	case USB_ST_TRANSFERRED:
		sc->sc_transfer.data_rem -= xfer->actlen;
		sc->sc_transfer.data_ptr += xfer->actlen;
		sc->sc_transfer.actlen += xfer->actlen;

		if (xfer->actlen < xfer->sumlen) {
			/* short transfer */
			sc->sc_transfer.data_rem = 0;
		}
	case USB_ST_SETUP:
		DPRINTF(sc, UDMASS_BBB, "max_bulk=%d, data_rem=%d\n",
		    max_bulk, sc->sc_transfer.data_rem);

		if (sc->sc_transfer.data_rem == 0) {
			umass_transfer_start(sc, UMASS_T_BBB_STATUS);
			return;
		}
		if (max_bulk > sc->sc_transfer.data_rem) {
			max_bulk = sc->sc_transfer.data_rem;
		}
		xfer->timeout = sc->sc_transfer.data_timeout;
		xfer->frlengths[0] = max_bulk;

		if (xfer->flags2.ext_buffer) {
			usb2_set_frame_data(xfer, sc->sc_transfer.data_ptr, 0);
		} else {
			usb2_copy_in(xfer->frbuffers, 0,
			    sc->sc_transfer.data_ptr, max_bulk);
		}

		usb2_start_hardware(xfer);
		return;

	default:			/* Error */
		if (xfer->error == USB_ERR_CANCELLED) {
			umass_tr_error(xfer);
		} else {
			umass_transfer_start(sc, UMASS_T_BBB_DATA_WR_CS);
		}
		return;

	}
}

void umass_t_bbb_data_wr_cs_callback(struct usbd_xfer *xfer)
{
	umass_t_bbb_data_clear_stall_callback(xfer, UMASS_T_BBB_STATUS,
	    UMASS_T_BBB_DATA_WRITE);
	return;
}

void umass_t_bbb_status_callback(struct usbd_xfer *xfer)
{
	struct umass_softc *sc = xfer->priv_sc;
	union ccb *ccb = sc->sc_transfer.ccb;
	uint32_t residue;

	switch (USB_GET_STATE(xfer)) {
	case USB_ST_TRANSFERRED:

		/*
		 * Do a full reset if there is something wrong with the CSW:
		 */
		sc->sc_status_try = 1;

		/* Zero missing parts of the CSW: */

		if (xfer->actlen < sizeof(sc->csw)) {
			bzero(&sc->csw, sizeof(sc->csw));
		}
		usb2_copy_out(xfer->frbuffers, 0, &sc->csw, xfer->actlen);

		DIF(UDMASS_BBB, umass_bbb_dump_csw(sc, &sc->csw));

		residue = UGETDW(sc->csw.dCSWDataResidue);

		if (!residue) {
			residue = (sc->sc_transfer.data_len -
			    sc->sc_transfer.actlen);
		}
		if (residue > sc->sc_transfer.data_len) {
			DPRINTF(sc, UDMASS_BBB, "truncating residue from %d "
			    "to %d bytes\n", residue, sc->sc_transfer.data_len);
			residue = sc->sc_transfer.data_len;
		}
		/* translate weird command-status signatures: */
		if (sc->sc_quirks & WRONG_CSWSIG) {

			uint32_t temp = UGETDW(sc->csw.dCSWSignature);

			if ((temp == CSWSIGNATURE_OLYMPUS_C1) ||
			    (temp == CSWSIGNATURE_IMAGINATION_DBX1)) {
				USETDW(sc->csw.dCSWSignature, CSWSIGNATURE);
			}
		}
		/* check CSW and handle eventual error */
		if (UGETDW(sc->csw.dCSWSignature) != CSWSIGNATURE) {
			DPRINTF(sc, UDMASS_BBB, "bad CSW signature 0x%08x != 0x%08x\n",
			    UGETDW(sc->csw.dCSWSignature), CSWSIGNATURE);
			/*
			 * Invalid CSW: Wrong signature or wrong tag might
			 * indicate that we lost synchronization. Reset the
			 * device.
			 */
			goto tr_error;
		} else if (UGETDW(sc->csw.dCSWTag) != UGETDW(sc->cbw.dCBWTag)) {
			DPRINTF(sc, UDMASS_BBB, "Invalid CSW: tag 0x%08x should be "
			    "0x%08x\n", UGETDW(sc->csw.dCSWTag),
			    UGETDW(sc->cbw.dCBWTag));
			goto tr_error;
		} else if (sc->csw.bCSWStatus > CSWSTATUS_PHASE) {
			DPRINTF(sc, UDMASS_BBB, "Invalid CSW: status %d > %d\n",
			    sc->csw.bCSWStatus, CSWSTATUS_PHASE);
			goto tr_error;
		} else if (sc->csw.bCSWStatus == CSWSTATUS_PHASE) {
			DPRINTF(sc, UDMASS_BBB, "Phase error, residue = "
			    "%d\n", residue);
			goto tr_error;
		} else if (sc->sc_transfer.actlen > sc->sc_transfer.data_len) {
			DPRINTF(sc, UDMASS_BBB, "Buffer overrun %d > %d\n",
			    sc->sc_transfer.actlen, sc->sc_transfer.data_len);
			goto tr_error;
		} else if (sc->csw.bCSWStatus == CSWSTATUS_FAILED) {
			DPRINTF(sc, UDMASS_BBB, "Command failed, residue = "
			    "%d\n", residue);

			sc->sc_transfer.ccb = NULL;

			sc->sc_last_xfer_index = UMASS_T_BBB_COMMAND;

			(sc->sc_transfer.callback)
			    (sc, ccb, residue, STATUS_CMD_FAILED);
		} else {
			sc->sc_transfer.ccb = NULL;

			sc->sc_last_xfer_index = UMASS_T_BBB_COMMAND;

			(sc->sc_transfer.callback)
			    (sc, ccb, residue, STATUS_CMD_OK);
		}
		return;

	case USB_ST_SETUP:
		xfer->frlengths[0] = xfer->max_data_length;
		usb2_start_hardware(xfer);
		return;

	default:
tr_error:
		DPRINTF(sc, UDMASS_BBB, "Failed to read CSW: %s, try %d\n",
		    usb2_errstr(xfer->error), sc->sc_status_try);

		if ((xfer->error == USB_ERR_CANCELLED) ||
		    (sc->sc_status_try)) {
			umass_tr_error(xfer);
		} else {
			sc->sc_status_try = 1;
			umass_transfer_start(sc, UMASS_T_BBB_DATA_RD_CS);
		}
		return;

	}
}

