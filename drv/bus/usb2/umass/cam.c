
#include "../netbas.h"
#include <usb_port.h>
#include <usb.h>
#include <usb_subr.h>

#include "umass.h"

void umass_cam_cb(struct umass_softc *sc, union ccb *ccb, uint32_t residue, uint8_t status);
static int umass_cam_attach_sim(struct umass_softc *sc);
static void umass_cam_rescan_callback(struct cam_periph *periph, union ccb *ccb);
static void umass_cam_rescan(struct umass_softc *sc);
static void umass_cam_attach(struct umass_softc *sc);
static void umass_cam_detach_sim(struct umass_softc *sc);
static void umass_cam_action(struct cam_sim *sim, union ccb *ccb);
static void umass_cam_poll(struct cam_sim *sim);
static void umass_cam_sense_cb(struct umass_softc *sc, union ccb *ccb, uint32_t residue, uint8_t status);
/*
 * CAM specific functions (used by SCSI, UFI, 8070i (ATAPI))
 */

static int
umass_cam_attach_sim(struct umass_softc *sc)
{
	struct cam_devq *devq;		/* Per device Queue */

	if (umass_sim[sc->sc_unit] != NULL) {
		sc->sc_sim = umass_sim[sc->sc_unit];
		goto register_only;
	}
	/*
	 * A HBA is attached to the CAM layer.
	 *
	 * The CAM layer will then after a while start probing for devices on
	 * the bus. The number of SIMs is limited to one.
	 */

	devq = cam_simq_alloc(1 /* maximum openings */ );
	if (devq == NULL) {
		return (ENOMEM);
	}
	sc->sc_sim = cam_sim_alloc
	    (&umass_cam_action, &umass_cam_poll,
	    DEVNAME_SIM,
	    sc /* priv */ ,
	    sc->sc_unit /* unit number */ ,
#if (__FreeBSD_version >= 700037)
	    &umass_mtx /* mutex */ ,
#endif
	    1 /* maximum device openings */ ,
	    0 /* maximum tagged device openings */ ,
	    devq);

	if (sc->sc_sim == NULL) {
		cam_simq_free(devq);
		return (ENOMEM);
	}
	umass_sim[sc->sc_unit] = sc->sc_sim;

register_only:

	/* update the softc pointer */
	sc->sc_sim->softc = sc;

#if (__FreeBSD_version >= 700037)
	mtx_lock(&umass_mtx);
#endif

#if (__FreeBSD_version >= 700048)
	if (xpt_bus_register(sc->sc_sim, sc->sc_dev, sc->sc_unit) != CAM_SUCCESS) {
		mtx_unlock(&umass_mtx);
		return (ENOMEM);
	}
#else
	if (xpt_bus_register(sc->sc_sim, sc->sc_unit) != CAM_SUCCESS) {
#if (__FreeBSD_version >= 700037)
		mtx_unlock(&umass_mtx);
#endif
		return (ENOMEM);
	}
#endif

#if (__FreeBSD_version >= 700037)
	mtx_unlock(&umass_mtx);
#endif
	return (0);
}

void umass_cam_rescan_callback(struct cam_periph *periph, union ccb *ccb)
{
#if USB_DEBUG
	struct umass_softc *sc = NULL;

	if (ccb->ccb_h.status != CAM_REQ_CMP) {
		DPRINTF(sc, UDMASS_SCSI, "%s:%d Rescan failed, 0x%04x\n",
		    periph->periph_name, periph->unit_number,
		    ccb->ccb_h.status);
	} else {
		DPRINTF(sc, UDMASS_SCSI, "%s%d: Rescan succeeded\n",
		    periph->periph_name, periph->unit_number);
	}
#endif

	xpt_free_path(ccb->ccb_h.path);
	free(ccb, M_USBDEV);
	return;
}

void umass_cam_rescan(struct umass_softc *sc)
{
	struct cam_path *path;
	union ccb *ccb;

	DPRINTF(sc, UDMASS_SCSI, "scbus%d: scanning for %d:%d:%d\n",
	    cam_sim_path(sc->sc_sim),
	    cam_sim_path(sc->sc_sim),
	    sc->sc_unit, CAM_LUN_WILDCARD);

	ccb = malloc(sizeof(*ccb), M_USBDEV, M_WAITOK | M_ZERO);

	if (ccb == NULL) {
		return;
	}
#if (__FreeBSD_version >= 700037)
	mtx_lock(&umass_mtx);
#endif

	if (xpt_create_path(&path, xpt_periph, cam_sim_path(sc->sc_sim),
	    CAM_TARGET_WILDCARD, CAM_LUN_WILDCARD)
	    != CAM_REQ_CMP) {
#if (__FreeBSD_version >= 700037)
		mtx_unlock(&umass_mtx);
#endif
		free(ccb, M_USBDEV);
		return;
	}
	xpt_setup_ccb(&ccb->ccb_h, path, 5 /* priority (low) */ );
	ccb->ccb_h.func_code = XPT_SCAN_BUS;
	ccb->ccb_h.cbfcnp = &umass_cam_rescan_callback;
	ccb->crcn.flags = CAM_FLAG_NONE;
	xpt_action(ccb);

#if (__FreeBSD_version >= 700037)
	mtx_unlock(&umass_mtx);
#endif

	/* The scan is in progress now. */

	return;
}

void umass_cam_attach(struct umass_softc *sc)
{
#ifndef USB_DEBUG
	if (bootverbose)
#endif
		printf("%s:%d:%d:%d: Attached to scbus%d\n",
		    sc->sc_name, cam_sim_path(sc->sc_sim),
		    sc->sc_unit, CAM_LUN_WILDCARD,
		    cam_sim_path(sc->sc_sim));

	if (!cold) {
		/*
		 * Notify CAM of the new device after a short delay. Any
		 * failure is benign, as the user can still do it by hand
		 * (camcontrol rescan <busno>). Only do this if we are not
		 * booting, because CAM does a scan after booting has
		 * completed, when interrupts have been enabled.
		 */

		/* scan the new sim */
		umass_cam_rescan(sc);
	}
	return;
}

/* umass_cam_detach
 *	detach from the CAM layer
 */

void umass_cam_detach_sim(struct umass_softc *sc)
{
	if (sc->sc_sim) {
		if (xpt_bus_deregister(cam_sim_path(sc->sc_sim))) {
#if 0					/* NOTYET */
			cam_sim_free(sc->sc_sim, /* free_devq */ TRUE);
#else
			/* accessing the softc is not possible after this */
			sc->sc_sim->softc = UMASS_GONE;
#endif
		} else {
			panic("%s: CAM layer is busy!\n",
			    sc->sc_name);
		}
		sc->sc_sim = NULL;
	}
	return;
}

/* umass_cam_action
 * 	CAM requests for action come through here
 */

void umass_cam_action(struct cam_sim *sim, union ccb *ccb)
{
	struct umass_softc *sc = (struct umass_softc *)sim->softc;

	if (sc == UMASS_GONE) {
		ccb->ccb_h.status = CAM_TID_INVALID;
		xpt_done(ccb);
		return;
	}
	if (sc) {
#if (__FreeBSD_version < 700037)
		mtx_lock(&umass_mtx);
#endif
	}
	/*
	 * Verify, depending on the operation to perform, that we either got
	 * a valid sc, because an existing target was referenced, or
	 * otherwise the SIM is addressed.
	 *
	 * This avoids bombing out at a printf and does give the CAM layer some
	 * sensible feedback on errors.
	 */
	switch (ccb->ccb_h.func_code) {
	case XPT_SCSI_IO:
	case XPT_RESET_DEV:
	case XPT_GET_TRAN_SETTINGS:
	case XPT_SET_TRAN_SETTINGS:
	case XPT_CALC_GEOMETRY:
		/* the opcodes requiring a target. These should never occur. */
		if (sc == NULL) {
			DPRINTF(sc, UDMASS_GEN, "%s:%d:%d:%d:func_code 0x%04x: "
			    "Invalid target (target needed)\n",
			    DEVNAME_SIM, cam_sim_path(sc->sc_sim),
			    ccb->ccb_h.target_id, ccb->ccb_h.target_lun,
			    ccb->ccb_h.func_code);

			ccb->ccb_h.status = CAM_TID_INVALID;
			xpt_done(ccb);
			goto done;
		}
		break;
	case XPT_PATH_INQ:
	case XPT_NOOP:
		/*
		 * The opcodes sometimes aimed at a target (sc is valid),
		 * sometimes aimed at the SIM (sc is invalid and target is
		 * CAM_TARGET_WILDCARD)
		 */
		if ((sc == NULL) &&
		    (ccb->ccb_h.target_id != CAM_TARGET_WILDCARD)) {
			DPRINTF(sc, UDMASS_SCSI, "%s:%d:%d:%d:func_code 0x%04x: "
			    "Invalid target (no wildcard)\n",
			    DEVNAME_SIM, cam_sim_path(sc->sc_sim),
			    ccb->ccb_h.target_id, ccb->ccb_h.target_lun,
			    ccb->ccb_h.func_code);

			ccb->ccb_h.status = CAM_TID_INVALID;
			xpt_done(ccb);
			goto done;
		}
		break;
	default:
		/* XXX Hm, we should check the input parameters */
		break;
	}

	/* Perform the requested action */
	switch (ccb->ccb_h.func_code) {
	case XPT_SCSI_IO:
		{
			uint8_t *cmd;
			uint8_t dir;

			if (ccb->csio.ccb_h.flags & CAM_CDB_POINTER) {
				cmd = (uint8_t *)(ccb->csio.cdb_io.cdb_ptr);
			} else {
				cmd = (uint8_t *)(ccb->csio.cdb_io.cdb_bytes);
			}

			DPRINTF(sc, UDMASS_SCSI, "%d:%d:%d:XPT_SCSI_IO: "
			    "cmd: 0x%02x, flags: 0x%02x, "
			    "%db cmd/%db data/%db sense\n",
			    cam_sim_path(sc->sc_sim), ccb->ccb_h.target_id,
			    ccb->ccb_h.target_lun, cmd[0],
			    ccb->ccb_h.flags & CAM_DIR_MASK, ccb->csio.cdb_len,
			    ccb->csio.dxfer_len, ccb->csio.sense_len);

			if (sc->sc_transfer.ccb) {
				DPRINTF(sc, UDMASS_SCSI, "%d:%d:%d:XPT_SCSI_IO: "
				    "I/O in progress, deferring\n",
				    cam_sim_path(sc->sc_sim), ccb->ccb_h.target_id,
				    ccb->ccb_h.target_lun);
				ccb->ccb_h.status = CAM_SCSI_BUSY;
				xpt_done(ccb);
				goto done;
			}
			switch (ccb->ccb_h.flags & CAM_DIR_MASK) {
			case CAM_DIR_IN:
				dir = DIR_IN;
				break;
			case CAM_DIR_OUT:
				dir = DIR_OUT;
				DIF(UDMASS_SCSI,
				    umass_dump_buffer(sc, ccb->csio.data_ptr,
				    ccb->csio.dxfer_len, 48));
				break;
			default:
				dir = DIR_NONE;
			}

			ccb->ccb_h.status = CAM_REQ_INPROG | CAM_SIM_QUEUED;

			/*
			 * sc->sc_transform will convert the command to the
			 * command format needed by the specific command set
			 * and return the converted command in
			 * "sc->sc_transfer.cmd_data"
			 */
			if (umass_std_transform(sc, ccb, cmd, ccb->csio.cdb_len)) {

				if (sc->sc_transfer.cmd_data[0] == INQUIRY) {

					/*
					 * Handle EVPD inquiry for broken devices first
					 * NO_INQUIRY also implies NO_INQUIRY_EVPD
					 */
					if ((sc->sc_quirks & (NO_INQUIRY_EVPD | NO_INQUIRY)) &&
					    (sc->sc_transfer.cmd_data[1] & SI_EVPD)) {
						struct scsi_sense_data *sense;

						sense = &ccb->csio.sense_data;
						bzero(sense, sizeof(*sense));
						sense->error_code = SSD_CURRENT_ERROR;
						sense->flags = SSD_KEY_ILLEGAL_REQUEST;
						sense->add_sense_code = 0x24;
						sense->extra_len = 10;
						ccb->csio.scsi_status = SCSI_STATUS_CHECK_COND;
						ccb->ccb_h.status = CAM_SCSI_STATUS_ERROR |
						    CAM_AUTOSNS_VALID;
						xpt_done(ccb);
						goto done;
					}
					/*
					 * Return fake inquiry data for
					 * broken devices
					 */
					if (sc->sc_quirks & NO_INQUIRY) {
						memcpy(ccb->csio.data_ptr, &fake_inq_data,
						    sizeof(fake_inq_data));
						ccb->csio.scsi_status = SCSI_STATUS_OK;
						ccb->ccb_h.status = CAM_REQ_CMP;
						xpt_done(ccb);
						goto done;
					}
					if (sc->sc_quirks & FORCE_SHORT_INQUIRY) {
						ccb->csio.dxfer_len = SHORT_INQUIRY_LENGTH;
					}
				} else if (sc->sc_transfer.cmd_data[0] == SYNCHRONIZE_CACHE) {
					if (sc->sc_quirks & NO_SYNCHRONIZE_CACHE) {
						ccb->csio.scsi_status = SCSI_STATUS_OK;
						ccb->ccb_h.status = CAM_REQ_CMP;
						xpt_done(ccb);
						goto done;
					}
				}
				umass_command_start(sc, dir, ccb->csio.data_ptr,
				    ccb->csio.dxfer_len,
				    ccb->ccb_h.timeout,
				    &umass_cam_cb, ccb);
			}
			break;
		}
	case XPT_PATH_INQ:
		{
			struct ccb_pathinq *cpi = &ccb->cpi;

			DPRINTF(sc, UDMASS_SCSI, "%d:%d:%d:XPT_PATH_INQ:.\n",
			    sc ? cam_sim_path(sc->sc_sim) : -1, ccb->ccb_h.target_id,
			    ccb->ccb_h.target_lun);

			/* host specific information */
			cpi->version_num = 1;
			cpi->hba_inquiry = 0;
			cpi->target_sprt = 0;
			cpi->hba_misc = PIM_NO_6_BYTE;
			cpi->hba_eng_cnt = 0;
			cpi->max_target = UMASS_SCSIID_MAX;	/* one target */
			cpi->initiator_id = UMASS_SCSIID_HOST;
			strlcpy(cpi->sim_vid, "FreeBSD", SIM_IDLEN);
			strlcpy(cpi->hba_vid, "USB SCSI", HBA_IDLEN);
			strlcpy(cpi->dev_name, cam_sim_name(sim), DEV_IDLEN);
			cpi->unit_number = cam_sim_unit(sim);
			cpi->bus_id = sc->sc_unit;
#if (__FreeBSD_version >= 700025)
			cpi->protocol = PROTO_SCSI;
			cpi->protocol_version = SCSI_REV_2;
			cpi->transport = XPORT_USB;
			cpi->transport_version = 0;
#endif
			if (sc == NULL) {
				cpi->base_transfer_speed = 0;
				cpi->max_lun = 0;
			} else {
				if (sc->sc_quirks & FLOPPY_SPEED) {
					cpi->base_transfer_speed =
					    UMASS_FLOPPY_TRANSFER_SPEED;
				} else if (usb2_get_speed(sc->sc_udev) ==
				    USB_SPEED_HIGH) {
					cpi->base_transfer_speed =
					    UMASS_HIGH_TRANSFER_SPEED;
				} else {
					cpi->base_transfer_speed =
					    UMASS_FULL_TRANSFER_SPEED;
				}
				cpi->max_lun = sc->sc_maxlun;
			}

			cpi->ccb_h.status = CAM_REQ_CMP;
			xpt_done(ccb);
			break;
		}
	case XPT_RESET_DEV:
		{
			DPRINTF(sc, UDMASS_SCSI, "%d:%d:%d:XPT_RESET_DEV:.\n",
			    cam_sim_path(sc->sc_sim), ccb->ccb_h.target_id,
			    ccb->ccb_h.target_lun);

			umass_reset(sc);

			ccb->ccb_h.status = CAM_REQ_CMP;
			xpt_done(ccb);
			break;
		}
	case XPT_GET_TRAN_SETTINGS:
		{
			struct ccb_trans_settings *cts = &ccb->cts;

			DPRINTF(sc, UDMASS_SCSI, "%d:%d:%d:XPT_GET_TRAN_SETTINGS:.\n",
			    cam_sim_path(sc->sc_sim), ccb->ccb_h.target_id,
			    ccb->ccb_h.target_lun);

#if (__FreeBSD_version >= 700025)
			cts->protocol = PROTO_SCSI;
			cts->protocol_version = SCSI_REV_2;
			cts->transport = XPORT_USB;
			cts->transport_version = 0;
			cts->xport_specific.valid = 0;
#else
			cts->valid = 0;
			cts->flags = 0;	/* no disconnection, tagging */
#endif
			ccb->ccb_h.status = CAM_REQ_CMP;
			xpt_done(ccb);
			break;
		}
	case XPT_SET_TRAN_SETTINGS:
		{
			DPRINTF(sc, UDMASS_SCSI, "%d:%d:%d:XPT_SET_TRAN_SETTINGS:.\n",
			    cam_sim_path(sc->sc_sim), ccb->ccb_h.target_id,
			    ccb->ccb_h.target_lun);

			ccb->ccb_h.status = CAM_FUNC_NOTAVAIL;
			xpt_done(ccb);
			break;
		}
	case XPT_CALC_GEOMETRY:
		{
			cam_calc_geometry(&ccb->ccg, /* extended */ 1);
			xpt_done(ccb);
			break;
		}
	case XPT_NOOP:
		{
			DPRINTF(sc, UDMASS_SCSI, "%d:%d:%d:XPT_NOOP:.\n",
			    sc ? cam_sim_path(sc->sc_sim) : -1, ccb->ccb_h.target_id,
			    ccb->ccb_h.target_lun);

			ccb->ccb_h.status = CAM_REQ_CMP;
			xpt_done(ccb);
			break;
		}
	default:
		DPRINTF(sc, UDMASS_SCSI, "%d:%d:%d:func_code 0x%04x: "
		    "Not implemented\n",
		    sc ? cam_sim_path(sc->sc_sim) : -1, ccb->ccb_h.target_id,
		    ccb->ccb_h.target_lun, ccb->ccb_h.func_code);

		ccb->ccb_h.status = CAM_FUNC_NOTAVAIL;
		xpt_done(ccb);
		break;
	}

done:
#if (__FreeBSD_version < 700037)
	if (sc) {
		mtx_unlock(&umass_mtx);
	}
#endif
	return;
}

void umass_cam_poll(struct cam_sim *sim)
{
	struct umass_softc *sc = (struct umass_softc *)sim->softc;

	if (sc == UMASS_GONE)
		return;

	DPRINTF(sc, UDMASS_SCSI, "CAM poll\n");

	usb2_do_poll(sc->sc_xfer, UMASS_T_MAX);
	return;
}


/* umass_cam_cb
 *	finalise a completed CAM command
 */

void umass_cam_cb(struct umass_softc *sc, union ccb *ccb, uint32_t residue,
    uint8_t status)
{
	ccb->csio.resid = residue;

	switch (status) {
	case STATUS_CMD_OK:
		ccb->ccb_h.status = CAM_REQ_CMP;
		if ((sc->sc_quirks & READ_CAPACITY_OFFBY1) &&
		    (ccb->ccb_h.func_code == XPT_SCSI_IO) &&
		    (ccb->csio.cdb_io.cdb_bytes[0] == READ_CAPACITY)) {
			struct scsi_read_capacity_data *rcap;
			uint32_t maxsector;

			rcap = (void *)(ccb->csio.data_ptr);
			maxsector = scsi_4btoul(rcap->addr) - 1;
			scsi_ulto4b(maxsector, rcap->addr);
		}
		xpt_done(ccb);
		break;

	case STATUS_CMD_UNKNOWN:
	case STATUS_CMD_FAILED:

		/* fetch sense data */

		/* the rest of the command was filled in at attach */
		sc->cam_scsi_sense.length = ccb->csio.sense_len;

		DPRINTF(sc, UDMASS_SCSI, "Fetching %d bytes of "
		    "sense data\n", ccb->csio.sense_len);

		if (umass_std_transform(sc, ccb, &sc->cam_scsi_sense.opcode,
		    sizeof(sc->cam_scsi_sense))) {

			if ((sc->sc_quirks & FORCE_SHORT_INQUIRY) &&
			    (sc->sc_transfer.cmd_data[0] == INQUIRY)) {
				ccb->csio.sense_len = SHORT_INQUIRY_LENGTH;
			}
			umass_command_start(sc, DIR_IN, &ccb->csio.sense_data.error_code,
			    ccb->csio.sense_len, ccb->ccb_h.timeout,
			    &umass_cam_sense_cb, ccb);
		}
		break;

	default:
		/*
		 * the wire protocol failed and will have recovered
		 * (hopefully).  We return an error to CAM and let CAM retry
		 * the command if necessary.
		 */
		ccb->ccb_h.status = CAM_REQ_CMP_ERR;
		xpt_done(ccb);
		break;
	}
}

/*
 * Finalise a completed autosense operation
 */
void umass_cam_sense_cb(struct umass_softc *sc, union ccb *ccb, uint32_t residue,
    uint8_t status)
{
	uint8_t *cmd;
	uint8_t key;

	switch (status) {
	case STATUS_CMD_OK:
	case STATUS_CMD_UNKNOWN:
	case STATUS_CMD_FAILED:

		if (ccb->csio.ccb_h.flags & CAM_CDB_POINTER) {
			cmd = (uint8_t *)(ccb->csio.cdb_io.cdb_ptr);
		} else {
			cmd = (uint8_t *)(ccb->csio.cdb_io.cdb_bytes);
		}

		key = (ccb->csio.sense_data.flags & SSD_KEY);

		/*
		 * Getting sense data always succeeds (apart from wire
		 * failures):
		 */
		if ((sc->sc_quirks & RS_NO_CLEAR_UA) &&
		    (cmd[0] == INQUIRY) &&
		    (key == SSD_KEY_UNIT_ATTENTION)) {
			/*
			 * Ignore unit attention errors in the case where
			 * the Unit Attention state is not cleared on
			 * REQUEST SENSE. They will appear again at the next
			 * command.
			 */
			ccb->ccb_h.status = CAM_REQ_CMP;
		} else if (key == SSD_KEY_NO_SENSE) {
			/*
			 * No problem after all (in the case of CBI without
			 * CCI)
			 */
			ccb->ccb_h.status = CAM_REQ_CMP;
		} else if ((sc->sc_quirks & RS_NO_CLEAR_UA) &&
			    (cmd[0] == READ_CAPACITY) &&
		    (key == SSD_KEY_UNIT_ATTENTION)) {
			/*
			 * Some devices do not clear the unit attention error
			 * on request sense. We insert a test unit ready
			 * command to make sure we clear the unit attention
			 * condition, then allow the retry to proceed as
			 * usual.
			 */

			ccb->ccb_h.status = CAM_SCSI_STATUS_ERROR
			    | CAM_AUTOSNS_VALID;
			ccb->csio.scsi_status = SCSI_STATUS_CHECK_COND;

#if 0
			DELAY(300000);
#endif
			DPRINTF(sc, UDMASS_SCSI, "Doing a sneaky"
			    "SCSI_TEST_UNIT_READY\n");

			/* the rest of the command was filled in at attach */

			if (umass_std_transform(sc, ccb,
			    &sc->cam_scsi_test_unit_ready.opcode,
			    sizeof(sc->cam_scsi_test_unit_ready))) {
				umass_command_start(sc, DIR_NONE, NULL, 0,
				    ccb->ccb_h.timeout,
				    &umass_cam_quirk_cb, ccb);
			}
			break;
		} else {
			ccb->ccb_h.status = CAM_SCSI_STATUS_ERROR
			    | CAM_AUTOSNS_VALID;
			ccb->csio.scsi_status = SCSI_STATUS_CHECK_COND;
		}
		xpt_done(ccb);
		break;

	default:
		DPRINTF(sc, UDMASS_SCSI, "Autosense failed, "
		    "status %d\n", status);
		ccb->ccb_h.status = CAM_AUTOSENSE_FAIL;
		xpt_done(ccb);
	}
	return;
}

/*
 * This completion code just handles the fact that we sent a test-unit-ready
 * after having previously failed a READ CAPACITY with CHECK_COND.  Even
 * though this command succeeded, we have to tell CAM to retry.
 */
void umass_cam_quirk_cb(struct umass_softc *sc, union ccb *ccb, uint32_t residue,
    uint8_t status)
{
	DPRINTF(sc, UDMASS_SCSI, "Test unit ready "
	    "returned status %d\n", status);

	ccb->ccb_h.status = CAM_SCSI_STATUS_ERROR
	    | CAM_AUTOSNS_VALID;
	ccb->csio.scsi_status = SCSI_STATUS_CHECK_COND;
	xpt_done(ccb);
	return;
}

