
#include "../netbas.h"
#include <usb_port.h>
#include <usb.h>
#include <usb_subr.h>

#include "umass.h"
void umass_cam_cb(struct umass_softc *sc, union ccb *ccb, uint32_t residue, uint8_t status);

/*
 * Command/Bulk/Interrupt (CBI) specific functions
 */

void umass_cbi_start_status(struct umass_softc *sc)
{
	if (sc->sc_xfer[UMASS_T_CBI_STATUS]) {
		umass_transfer_start(sc, UMASS_T_CBI_STATUS);
	} else {
		union ccb *ccb = sc->sc_transfer.ccb;

		sc->sc_transfer.ccb = NULL;

		sc->sc_last_xfer_index = UMASS_T_CBI_COMMAND;

		(sc->sc_transfer.callback)
		    (sc, ccb, (sc->sc_transfer.data_len -
		    sc->sc_transfer.actlen), STATUS_CMD_UNKNOWN);
	}
	return;
}

void umass_t_cbi_reset1_callback(struct usbd_xfer *xfer)
{
	struct umass_softc *sc = xfer->priv_sc;
	struct usb_device_request req;
	uint8_t buf[UMASS_CBI_DIAGNOSTIC_CMDLEN];

	uint8_t i;

	switch (USB_GET_STATE(xfer)) {
	case USB_ST_TRANSFERRED:
		umass_transfer_start(sc, UMASS_T_CBI_RESET2);
		return;

	case USB_ST_SETUP:
		/*
		 * Command Block Reset Protocol
		 *
		 * First send a reset request to the device. Then clear
		 * any possibly stalled bulk endpoints.
		 *
		 * This is done in 3 steps, using 3 transfers:
		 * UMASS_T_CBI_RESET1
		 * UMASS_T_CBI_RESET2
		 * UMASS_T_CBI_RESET3
		 * UMASS_T_CBI_RESET4 (only if there is an interrupt endpoint)
		 */

		DPRINTF(sc, UDMASS_CBI, "CBI reset!\n");

		req.bmRequestType = UT_WRITE_CLASS_INTERFACE;
		req.bRequest = UR_CBI_ADSC;
		USETW(req.wValue, 0);
		req.wIndex[0] = sc->sc_iface_no;
		req.wIndex[1] = 0;
		USETW(req.wLength, UMASS_CBI_DIAGNOSTIC_CMDLEN);

		/*
		 * The 0x1d code is the SEND DIAGNOSTIC command. To
		 * distinguish between the two, the last 10 bytes of the CBL
		 * is filled with 0xff (section 2.2 of the CBI
		 * specification)
		 */
		buf[0] = 0x1d;		/* Command Block Reset */
		buf[1] = 0x04;

		for (i = 2; i < UMASS_CBI_DIAGNOSTIC_CMDLEN; i++) {
			buf[i] = 0xff;
		}

		usb2_copy_in(xfer->frbuffers, 0, &req, sizeof(req));
		usb2_copy_in(xfer->frbuffers + 1, 0, buf, sizeof(buf));

		xfer->frlengths[0] = sizeof(req);
		xfer->frlengths[1] = sizeof(buf);
		xfer->nframes = 2;
		usb2_start_hardware(xfer);
		return;

	default:			/* Error */
		umass_tr_error(xfer);
		return;

	}
}

void umass_t_cbi_reset2_callback(struct usbd_xfer *xfer)
{
	umass_t_cbi_data_clear_stall_callback(xfer, UMASS_T_CBI_RESET3,
	    UMASS_T_CBI_DATA_READ);
	return;
}

void umass_t_cbi_reset3_callback(struct usbd_xfer *xfer)
{
	struct umass_softc *sc = xfer->priv_sc;

	umass_t_cbi_data_clear_stall_callback
	    (xfer, (sc->sc_xfer[UMASS_T_CBI_RESET4] &&
	    sc->sc_xfer[UMASS_T_CBI_STATUS]) ?
	    UMASS_T_CBI_RESET4 : UMASS_T_CBI_COMMAND,
	    UMASS_T_CBI_DATA_WRITE);

	return;
}

void umass_t_cbi_reset4_callback(struct usbd_xfer *xfer)
{
	umass_t_cbi_data_clear_stall_callback(xfer, UMASS_T_CBI_COMMAND,
	    UMASS_T_CBI_STATUS);
	return;
}

void umass_t_cbi_data_clear_stall_callback(struct usbd_xfer *xfer,
    uint8_t next_xfer,
    uint8_t stall_xfer)
{
	struct umass_softc *sc = xfer->priv_sc;

	switch (USB_GET_STATE(xfer)) {
	case USB_ST_TRANSFERRED:
tr_transferred:
		if (next_xfer == UMASS_T_CBI_STATUS) {
			umass_cbi_start_status(sc);
		} else {
			umass_transfer_start(sc, next_xfer);
		}
		return;

	case USB_ST_SETUP:
		if (usb2_clear_stall_callback(xfer, sc->sc_xfer[stall_xfer])) {
			goto tr_transferred;	/* should not happen */
		}
		return;

	default:			/* Error */
		umass_tr_error(xfer);
		return;

	}
}

void umass_t_cbi_command_callback(struct usbd_xfer *xfer)
{
	struct umass_softc *sc = xfer->priv_sc;
	union ccb *ccb = sc->sc_transfer.ccb;
	struct usb_device_request req;

	switch (USB_GET_STATE(xfer)) {
	case USB_ST_TRANSFERRED:

		if (sc->sc_transfer.dir == DIR_NONE) {
			umass_cbi_start_status(sc);
		} else {
			umass_transfer_start
			    (sc, (sc->sc_transfer.dir == DIR_IN) ?
			    UMASS_T_CBI_DATA_READ : UMASS_T_CBI_DATA_WRITE);
		}
		return;

	case USB_ST_SETUP:

		if (ccb) {

			/*
		         * do a CBI transfer with cmd_len bytes from
		         * cmd_data, possibly a data phase of data_len
		         * bytes from/to the device and finally a status
		         * read phase.
		         */

			req.bmRequestType = UT_WRITE_CLASS_INTERFACE;
			req.bRequest = UR_CBI_ADSC;
			USETW(req.wValue, 0);
			req.wIndex[0] = sc->sc_iface_no;
			req.wIndex[1] = 0;
			req.wLength[0] = sc->sc_transfer.cmd_len;
			req.wLength[1] = 0;

			usb2_copy_in(xfer->frbuffers, 0, &req, sizeof(req));
			usb2_copy_in(xfer->frbuffers + 1, 0, sc->sc_transfer.cmd_data,
			    sc->sc_transfer.cmd_len);

			xfer->frlengths[0] = sizeof(req);
			xfer->frlengths[1] = sc->sc_transfer.cmd_len;
			xfer->nframes = xfer->frlengths[1] ? 2 : 1;

			DIF(UDMASS_CBI,
			    umass_cbi_dump_cmd(sc,
			    sc->sc_transfer.cmd_data,
			    sc->sc_transfer.cmd_len));

			usb2_start_hardware(xfer);
		}
		return;

	default:			/* Error */
		umass_tr_error(xfer);
		return;

	}
}

void umass_t_cbi_data_read_callback(struct usbd_xfer *xfer)
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
		DPRINTF(sc, UDMASS_CBI, "max_bulk=%d, data_rem=%d\n",
		    max_bulk, sc->sc_transfer.data_rem);

		if (sc->sc_transfer.data_rem == 0) {
			umass_cbi_start_status(sc);
			return;
		}
		if (max_bulk > sc->sc_transfer.data_rem) {
			max_bulk = sc->sc_transfer.data_rem;
		}
		xfer->timeout = sc->sc_transfer.data_timeout;

		xfer->frlengths[0] = max_bulk;
		usb2_start_hardware(xfer);
		return;

	default:			/* Error */
		if ((xfer->error == USB_ERR_CANCELLED) ||
		    (sc->sc_transfer.callback != &umass_cam_cb)) {
			umass_tr_error(xfer);
		} else {
			umass_transfer_start(sc, UMASS_T_CBI_DATA_RD_CS);
		}
		return;

	}
}

void umass_t_cbi_data_rd_cs_callback(struct usbd_xfer *xfer)
{
	umass_t_cbi_data_clear_stall_callback(xfer, UMASS_T_CBI_STATUS,
	    UMASS_T_CBI_DATA_READ);
	return;
}

void umass_t_cbi_data_write_callback(struct usbd_xfer *xfer)
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
		DPRINTF(sc, UDMASS_CBI, "max_bulk=%d, data_rem=%d\n",
		    max_bulk, sc->sc_transfer.data_rem);

		if (sc->sc_transfer.data_rem == 0) {
			umass_cbi_start_status(sc);
			return;
		}
		if (max_bulk > sc->sc_transfer.data_rem) {
			max_bulk = sc->sc_transfer.data_rem;
		}
		xfer->timeout = sc->sc_transfer.data_timeout;

		if (xfer->flags2.ext_buffer) {
			usb2_set_frame_data(xfer, sc->sc_transfer.data_ptr, 0);
		} else {
			usb2_copy_in(xfer->frbuffers, 0,
			    sc->sc_transfer.data_ptr, max_bulk);
		}

		xfer->frlengths[0] = max_bulk;
		usb2_start_hardware(xfer);
		return;

	default:			/* Error */
		if ((xfer->error == USB_ERR_CANCELLED) ||
		    (sc->sc_transfer.callback != &umass_cam_cb)) {
			umass_tr_error(xfer);
		} else {
			umass_transfer_start(sc, UMASS_T_CBI_DATA_WR_CS);
		}
		return;

	}
}

void umass_t_cbi_data_wr_cs_callback(struct usbd_xfer *xfer)
{
	umass_t_cbi_data_clear_stall_callback(xfer, UMASS_T_CBI_STATUS,
	    UMASS_T_CBI_DATA_WRITE);
	return;
}

void umass_t_cbi_status_callback(struct usbd_xfer *xfer)
{
	struct umass_softc *sc = xfer->priv_sc;
	union ccb *ccb = sc->sc_transfer.ccb;
	uint32_t residue;
	uint8_t status;

	switch (USB_GET_STATE(xfer)) {
	case USB_ST_TRANSFERRED:

		if (xfer->actlen < sizeof(sc->sbl)) {
			goto tr_setup;
		}
		usb2_copy_out(xfer->frbuffers, 0, &sc->sbl, sizeof(sc->sbl));

		residue = (sc->sc_transfer.data_len -
		    sc->sc_transfer.actlen);

		/* dissect the information in the buffer */

		if (sc->sc_proto & UMASS_PROTO_UFI) {

			/*
			 * Section 3.4.3.1.3 specifies that the UFI command
			 * protocol returns an ASC and ASCQ in the interrupt
			 * data block.
			 */

			DPRINTF(sc, UDMASS_CBI, "UFI CCI, ASC = 0x%02x, "
			    "ASCQ = 0x%02x\n", sc->sbl.ufi.asc,
			    sc->sbl.ufi.ascq);

			status = (((sc->sbl.ufi.asc == 0) &&
			    (sc->sbl.ufi.ascq == 0)) ?
			    STATUS_CMD_OK : STATUS_CMD_FAILED);

			sc->sc_transfer.ccb = NULL;

			sc->sc_last_xfer_index = UMASS_T_CBI_COMMAND;

			(sc->sc_transfer.callback)
			    (sc, ccb, residue, status);

			return;

		} else {

			/* Command Interrupt Data Block */

			DPRINTF(sc, UDMASS_CBI, "type=0x%02x, value=0x%02x\n",
			    sc->sbl.common.type, sc->sbl.common.value);

			if (sc->sbl.common.type == IDB_TYPE_CCI) {

				status = (sc->sbl.common.value & IDB_VALUE_STATUS_MASK);

				status = ((status == IDB_VALUE_PASS) ? STATUS_CMD_OK :
				    (status == IDB_VALUE_FAIL) ? STATUS_CMD_FAILED :
				    (status == IDB_VALUE_PERSISTENT) ? STATUS_CMD_FAILED :
				    STATUS_WIRE_FAILED);

				sc->sc_transfer.ccb = NULL;

				sc->sc_last_xfer_index = UMASS_T_CBI_COMMAND;

				(sc->sc_transfer.callback)
				    (sc, ccb, residue, status);

				return;
			}
		}

		/* fallthrough */

	case USB_ST_SETUP:
tr_setup:
		xfer->frlengths[0] = xfer->max_data_length;
		usb2_start_hardware(xfer);
		return;

	default:			/* Error */
		DPRINTF(sc, UDMASS_CBI, "Failed to read CSW: %s\n",
		    usb2_errstr(xfer->error));
		umass_tr_error(xfer);
		return;

	}
}

