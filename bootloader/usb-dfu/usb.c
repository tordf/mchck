#include <sys/types.h>

#include <inttypes.h>
#include <string.h>
#include <wchar.h>

/**
 * Note: bitfields ahead.
 * GCC fills the fields lsb-to-msb on little endian.
 */

#define __packed __attribute__((__packed__))

/* From FreeBSD: compile-time asserts */
#define CTASSERT(x)             _CTASSERT(x, __LINE__)
#define _CTASSERT(x, y)         __CTASSERT(x, y)
#define __CTASSERT(x, y)        typedef char __assert ## y[(x) ? 1 : -1]

#define CTASSERT_SIZE_BYTE(t, s)     CTASSERT(sizeof(t) == (s))
#define CTASSERT_SIZE_BIT(t, s)     CTASSERT(sizeof(t) * 8 == (s))


/**
 * USB descriptors
 */

enum usb_desc_type {
	USB_DESC_DEV = 1,
	USB_DESC_CONFIG = 2,
	USB_DESC_STRING = 3,
	USB_DESC_IFACE = 4,
	USB_DESC_EP = 5,
	USB_DESC_DEVQUAL = 6,
	USB_DESC_OTHERSPEED = 7,
	USB_DESC_POWER = 8
};

enum usb_dev_class {
	USB_DEV_CLASS_SEE_IFACE = 0,
	USB_DEV_CLASS_VENDOR = 0xff
};

enum usb_dev_subclass {
	USB_DEV_SUBCLASS_SEE_IFACE = 0,
	USB_DEV_SUBCLASS_VENDOR = 0xff
};

enum usb_dev_proto {
	USB_DEV_PROTO_SEE_IFACE = 0,
	USB_DEV_PROTO_VENDOR = 0xff
};

union usb_bcd_t {
	struct {
		uint8_t sub : 4;
		uint8_t min : 4;
		uint16_t maj : 8;
	};
	uint16_t bcd;
};
CTASSERT_SIZE_BYTE(union usb_bcd_t, 2);

struct usb_desc_dev_t {
	uint8_t desc_length;
	enum usb_desc_type desc_type : 8; /* = USB_DESC_DEV */
	union usb_bcd_t usbver;	     /* = 0x0200 */
	enum usb_dev_class devclass : 8;
	enum usb_dev_subclass devsubclass : 8;
	enum usb_dev_proto : 8;
	uint8_t ep0_maxsize;
	uint16_t vid;
	uint16_t pid;
	union usb_bcd_t devver;
	uint8_t manuf_strdesc;
	uint8_t prod_strdesc;
	uint8_t serial_strdesc;
	uint8_t numconfig;
} __packed;
CTASSERT_SIZE_BYTE(struct usb_desc_dev_t, 18);

struct usb_desc_config_t {
	uint8_t desc_length;
	enum usb_desc_type desc_type : 8; /* = USB_DESC_CONFIG */
	uint16_t total_length;	     /* size of config, iface, ep */
	uint8_t num_ifaces;
	uint8_t config_val;
	uint8_t config_strdesc;
	struct {
		uint8_t _rsvd0 : 5;
		uint8_t remote_wakeup : 1;
		uint8_t self_powered : 1;
		uint8_t one : 1; /* = 1 for historical reasons */
	};
	uint8_t maxpower;	/* units of 2mA */
} __packed;
CTASSERT_SIZE_BYTE(struct usb_desc_config_t, 9);

enum usb_iface_class {
	USB_IFACE_CLASS_SEE_IFACE = 0,
	USB_IFACE_CLASS_VENDOR = 0xff
};

enum usb_iface_subclass {
	USB_IFACE_SUBCLASS_SEE_IFACE = 0,
	USB_IFACE_SUBCLASS_VENDOR = 0xff
};

enum usb_iface_proto {
	USB_IFACE_PROTO_SEE_IFACE = 0,
	USB_IFACE_PROTO_VENDOR = 0xff
};

struct usb_desc_iface_t {
	uint8_t desc_length;
	enum usb_desc_type desc_type : 8; /* = USB_DESC_IFACE */
	uint8_t iface_num;
	uint8_t alternate;
	uint8_t num_ep;
	enum usb_iface_class : 8;
	enum usb_iface_subclass : 8;
	enum usb_iface_proto : 8;
	uint8_t iface_strdesc;
} __packed;
CTASSERT_SIZE_BYTE(struct usb_desc_iface_t, 9);

struct usb_desc_ep_t {
	uint8_t desc_length;
	enum usb_desc_type desc_type : 8; /* = USB_DESC_EP */
	union {
		struct {
			uint8_t ep_num : 4;
			uint8_t _rsvd0 : 3;
			uint8_t in : 1;
		};
		uint8_t addr;
	};
	struct {
		enum usb_ep_type {
			USB_EP_CONTROL = 0,
			USB_EP_ISO = 1,
			USB_EP_BULK = 2,
			USB_EP_INTR = 3
		} type : 2;
		enum usb_ep_iso_synctype {
			USB_EP_ISO_NOSYNC = 0,
			USB_EP_ISO_ASYNC = 1,
			USB_EP_ISO_ADAPTIVE = 2,
			USB_EP_ISO_SYNC = 3
		} sync_type : 2;
		enum usb_ep_iso_usagetype {
			USB_EP_ISO_DATA = 0,
			USB_EP_ISO_FEEDBACK = 1,
			USB_EP_ISO_IMPLICIT = 2
		} usage_type : 2;
		uint8_t _rsvd1 : 2;
	} __packed;
	struct {
		uint16_t maxsize : 11;
		uint16_t _rsvd2 : 5;
	};
	uint8_t interval;
} __packed;
CTASSERT_SIZE_BYTE(struct usb_desc_ep_t, 7);

struct usb_desc_string_t {
	uint8_t desc_length;
	enum usb_desc_type desc_type : 8; /* = USB_DESC_STRING */
	wchar_t string[];
};


/**
 * USB request data structures.
 */

struct usb_ctrl_req_t {
	union /* reqtype and request & u16 */ {
		struct /* reqtype and request */ {
			union /* reqtype in bitfield & u8 */ {
				struct /* reqtype */ {
					enum usb_ctrl_req_recp {
						USB_CTRL_REQ_DEV = 0,
						USB_CTRL_REQ_IFACE = 1,
						USB_CTRL_REQ_EP = 2,
						USB_CTRL_REQ_OTHER = 3
					} recp : 5;
					enum usb_ctrl_req_type {
						USB_CTRL_REQ_STD = 0,
						USB_CTRL_REQ_CLASS = 1,
						USB_CTRL_REQ_VENDOR = 2
					} type : 2;
					enum usb_ctrl_req_dir {
						USB_CTRL_REQ_OUT = 0,
						USB_CTRL_REQ_IN = 1
					} in : 1;
				} __packed;
				uint8_t reqtype;
			}; /* union */
			enum usb_ctrl_req_code {
				USB_CTRL_REQ_GET_STATUS = 0,
				USB_CTRL_REQ_CLEAR_FEATURE = 1,
				USB_CTRL_REQ_SET_FEATURE = 3,
				USB_CTRL_REQ_SET_ADDRESS = 5,
				USB_CTRL_REQ_GET_DESCRIPTOR = 6,
				USB_CTRL_REQ_SET_DESCRIPTOR = 7,
				USB_CTRL_REQ_GET_CONFIGURATION = 8,
				USB_CTRL_REQ_SET_CONFIGURATION = 9,
				USB_CTRL_REQ_GET_INTERFACE = 10,
				USB_CTRL_REQ_SET_INTERFACE = 11,
				USB_CTRL_REQ_SYNC_FRAME = 12
			} request : 8;
		} __packed; /* struct */
		uint16_t type_and_req;
	}; /* union */
	uint16_t value;
	uint16_t index;
	uint16_t length;
} __packed;
CTASSERT_SIZE_BYTE(struct usb_ctrl_req_t, 8);

#define USB_CTRL_REQ_DIR_SHIFT 0
#define USB_CTRL_REQ_TYPE_SHIFT 1
#define USB_CTRL_REQ_RECP_SHIFT 3
#define USB_CTRL_REQ_CODE_SHIFT 8
#define USB_CTRL_REQ(req_inout, req_type, req_code)			\
	(uint16_t)							\
	((USB_CTRL_REQ_##req_inout << USB_CTRL_REQ_DIR_SHIFT)		\
	 |(USB_CTRL_REQ_##req_type << USB_CTRL_REQ_TYPE_SHIFT)		\
	 |(USB_CTRL_REQ_##req_code << USB_CTRL_REQ_CODE_SHIFT))

/**
 * status replies for GET_STATUS.
 */

struct usb_ctrl_req_status_dev_t {
	uint16_t self_powered : 1;
	uint16_t remote_wakeup : 1;
	uint16_t _rsvd : 14;
};
CTASSERT_SIZE_BIT(struct usb_ctrl_req_status_dev_t, 16);

struct usb_ctrl_req_status_iface_t {
	uint16_t _rsvd;
};
CTASSERT_SIZE_BIT(struct usb_ctrl_req_status_iface_t, 16);

struct usb_ctrl_req_status_ep_t {
	uint16_t halt : 1;
	uint16_t _rsvd : 15;
};
CTASSERT_SIZE_BIT(struct usb_ctrl_req_status_ep_t, 16);

/**
 * Descriptor type (in req->value) for GET_DESCRIPTOR.
 */
struct usb_ctrl_req_desc_t {
	uint8_t idx;
	enum usb_desc_type type : 8;
} __packed;
CTASSERT_SIZE_BIT(struct usb_ctrl_req_desc_t, 16);

/**
 * Feature selector (in req->value) for CLEAR_FEATURE.
 */
enum usb_ctrl_req_feature {
	USB_CTRL_REQ_FEAT_EP_HALT = 0,
	USB_CTRL_REQ_FEAT_DEV_REMOTE_WKUP = 1,
	USB_CTRL_REQ_FEAT_TEST_MODE = 2
};


/**
 * Hardware structures
 */

struct USB_BD_t {
	union /* bitfields */ {
		struct /* common */ {
			uint32_t _rsvd0	 : 6;
			enum usb_data01 {
				USB_DATA01_DATA0 = 0,
				USB_DATA01_DATA1 = 1
			} data01	 : 1;
			uint32_t own	 : 1;
			uint32_t _rsvd1	 : 8;
			uint32_t bc	 : 10;
			uint32_t _rsvd2	 : 6;
		} __packed;
		struct /* USB-FS */ {
			uint32_t _rsvd3	 : 2;
			uint32_t stall	 : 1;
			uint32_t dts	 : 1;
			uint32_t ninc	 : 1;
			uint32_t keep	 : 1;
			uint32_t _rsvd4	 : 26;
		} __packed;
		struct /* processor */ {
			uint32_t _rsvd5	 : 2;
			enum usb_tok_pid {
				USB_PID_TIMEOUT = 0,
				USB_PID_OUT = 1,
				USB_PID_ACK = 2,
				USB_PID_DATA0 = 3,
				USB_PID_IN = 9,
				USB_PID_NAK = 10,
				USB_PID_DATA1 = 11,
				USB_PID_SETUP = 13,
				USB_PID_STALL = 14,
				USB_PID_DATAERR = 15
			} tok_pid : 4;
			uint32_t _rsvd6	 : 26;
		} __packed;
		uint32_t bd;
	}; /* union bitfields */
	void *addr;
};
CTASSERT_SIZE_BYTE(struct USB_BD_t, 8);

struct USB_STAT_t {
	union {
		struct {
			uint8_t _rsvd0 : 2;
			enum usb_ep_pingpong {
				USB_EP_PINGPONG_EVEN = 0,
				USB_EP_PINGPONG_ODD = 1
			} pingpong : 1;
			enum usb_ep_dir {
				USB_EP_RX = 0,
				USB_EP_TX = 1
			} dir : 1;
			uint8_t ep : 4;
		};
		uint32_t stat;
	};
};
CTASSERT_SIZE_BIT(struct USB_STAT_t, 32);

struct USB_ENDPT_t {
	uint8_t ephshk : 1;
	uint8_t epstall : 1;
	uint8_t eptxen : 1;
	uint8_t eprxen : 1;
	uint8_t epctldis : 1;
	uint8_t _rsvd0 : 1;
	uint8_t retrydis : 1;
	uint8_t hostwohub : 1;
	uint32_t _rsvd1 : 24;
} __packed;
CTASSERT_SIZE_BIT(struct USB_ENDPT_t, 32);

struct USB_ADDR_t {
	uint8_t addr : 7;
	uint8_t lsen : 1;
} __packed;
CTASSERT_SIZE_BIT(struct USB_ADDR_t, 8);

struct USB_CTL_t {
	union {
		struct /* common */ {
			uint8_t _rsvd1 : 1;
			uint8_t oddrst : 1;
			uint8_t resume : 1;
			uint8_t _rsvd2 : 3;
			uint8_t se0 : 1;
			uint8_t jstate : 1;
		} __packed;
		struct /* host */ {
			uint8_t sofen : 1;
			uint8_t _rsvd3 : 2;
			uint8_t hostmodeen : 1;
			uint8_t reset : 1;
			uint8_t token_busy : 1;
			uint8_t _rsvd4 : 2;
		} __packed;
		struct /* device */ {
			uint8_t usben : 1;
			uint8_t _rsvd5 : 4;
			uint8_t txd_suspend : 1;
			uint8_t _rsvd6 : 2;
		} __packed;
	};
} __packed;
CTASSERT_SIZE_BIT(struct USB_CTL_t, 8);

extern volatile struct USB_ENDPT_t USB0_ENDPT[16];
extern volatile struct USB_ADDR_t USB0_ADDR;
extern volatile struct USB_CTL_t USB0_CTL;


/**
 * Internal driver structures
 */

/**
 * USB state machine
 * =================
 *
 * Device configuration states:
 *
 * Attached <-> Powered
 * Powered -(reset)-> Default
 * Default -(SET_ADDRESS)-> Address
 * Address -(SET_CONFIGURATION)-> Configured
 * Configured -(SET_CONFIGURATION 0)-> Address
 * Address -(SET_ADDRESS 0)-> Default
 * [Default, Configured, Address] -(reset)-> Default
 */


/**
 * Kinetis USB driver notes:
 * We need to manually maintain the DATA0/1 toggling for the SIE.
 * SETUP transactions always start with a DATA0.
 *
 * The SIE internally uses pingpong (double) buffering, which is
 * easily confused with DATA0/DATA1 toggling, and I think even the
 * Freescale docs confuse the two notions.  When BD->DTS is set,
 * BD->DATA01 will be used to verify/discard incoming DATAx and it
 * will set the DATAx PID for outgoing tokens.  This is not described
 * as such in the Freescale Kinetis docs, but the Microchip PIC32 OTG
 * docs are more clear on this;  it seems that both Freescale and
 * Microchip use different versions of the same USB OTG IP core.
 *
 * http://ww1.microchip.com/downloads/en/DeviceDoc/61126F.pdf
 *
 * Clear CTL->TOKEN_BUSY after SETUP tokens.
 */

#define EP0_BUFSIZE 64


enum usbd_dev_state {
	USBD_STATE_DISABLED = 0,
	USBD_STATE_DEFAULT,
	USBD_STATE_SETTING_ADDRESS,
	USBD_STATE_ADDRESS,
	USBD_STATE_CONFIGURED
};

typedef void (*ep_callback_t)(void *buf, ssize_t len, void *data);


struct usbd_ep_pipe_state_t {
	enum usb_ep_pingpong pingpong; /* next desc to use */
	enum usb_data01 data01;
	size_t transfer_size;
	size_t pos;
	uint8_t *data_buf;
	int short_transfer;
	ep_callback_t callback;
	void *callback_data;
	size_t ep_maxsize;
};

struct usbd_ep_state_t {
	union {
		struct usbd_ep_pipe_state_t pipe[2];
		struct {
			struct usbd_ep_pipe_state_t rx;
			struct usbd_ep_pipe_state_t tx;
		};
	};
};

struct usbd_t {
	struct USB_BD_t *bdt;
	enum usbd_dev_state state;
	enum usbd_ctrl_state {
		USBD_CTRL_STATE_IDLE,
		USBD_CTRL_STATE_DATA,
		USBD_CTRL_STATE_STATUS
	} ctrl_state;
	enum usb_ctrl_req_dir ctrl_dir;
	int address;
	int config;
	struct usbd_ep_state_t ep0_state;
	uint8_t ep0_buf[EP0_BUFSIZE][2];
};


struct usbd_t usb;


void
usb_enable(void)
{
	/* clock distribution? */
	/* INTEN->(TOKDNE,USBRST)=1 */
	/* BDTPAGE1,2,3 */
	/* ENDPT0->(EPRXEN,EPTXEN,EPHSHK)=1 */
	/* USBCTRL->(SUSP,PDE)=0 */
	/* CTL->USBENSOFEN=1 */
}

void
usb_intr(void)
{
	/* check STAT->(ENDP,TX,ODD) */
	/* check ERRSTAT->(DMAERR) */
	/* read BDT entry */
}

static struct USB_BD_t *
usb_get_bd(int ep, enum usb_ep_dir dir, enum usb_ep_pingpong pingpong)
{
	return (&usb.bdt[(ep << 2) | (dir << 1) | pingpong]);
}

static struct USB_BD_t *
usb_get_bd_stat(struct USB_STAT_t stat)
{
	return (((void *)(uintptr_t)usb.bdt + (stat.stat << 1)));
}

/**
 * Stalls the EP.  SETUP transactions automatically unstall an EP.
 */
void
usb_ep_stall(int ep)
{
	USB0_ENDPT[ep].epstall = 1;
}

static void
usb_tx_queue_next(struct usbd_ep_pipe_state_t *s)
{
	struct USB_BD_t *bd = usb_get_bd(0, USB_EP_TX, s->pingpong);
	size_t thislen = s->transfer_size;

	if (thislen > s->ep_maxsize)
		thislen = s->ep_maxsize;

	bd->addr = s->data_buf + s->pos;
	s->pos += thislen;
	s->transfer_size -= thislen;

	/* XXX fairly inefficient assignment */
	bd->bd = ((struct USB_BD_t) {
			.bc = thislen,
			.dts = 1,
			.data01 = s->data01,
			.own = 1
		}).bd;
	s->pingpong ^= 1;
}

/**
 * Returns: 0 when this is was the last transfer, 1 if there is still
 * more to go.
 */
/* Defaults to EP0 for now */
static int
usb_tx_next(void)
{
	struct usbd_ep_pipe_state_t *s = &usb.ep0_state.tx;

	/**
	 * Us being here means the previous transfer just completed
	 * successfully.  That means the host just toggled its data
	 * sync bit, and so do we.
	 */
	s->data01 ^= 1;

	if (s->transfer_size > 0) {
		usb_tx_queue_next(s);
		return (1);
	}

	/**
	 * All data has been shipped.  Do we need to send a short
	 * packet?
	 */
	if (s->short_transfer) {
		s->short_transfer = 0;
		usb_tx_queue_next(s);
		return (1);
	}

	if (s->callback)
		s->callback(s->data_buf, 0, s->callback_data);

	return (0);
}

/**
 * send USB data (IN device transaction)
 *
 * So far this function is specialized for EP 0 only.
 *
 * Returns: size to be transfered, or -1 on error.
 */
int
usb_tx(void *buf, size_t len, size_t reqlen, ep_callback_t cb, void *cb_data)
{
	struct usbd_ep_pipe_state_t *s = &usb.ep0_state.tx;

	s->data_buf = buf;
	s->transfer_size = len;
	s->pos = 0;
	s->callback = cb;
	s->callback_data = cb_data;
	if (s->transfer_size > reqlen)
		s->transfer_size = reqlen;
	if (s->transfer_size == reqlen)
		s->short_transfer = 0;
	else
		s->short_transfer = 1;

	usb_tx_queue_next(s);
	return (s->transfer_size);
}

int
usb_tx_cp(void *buf, size_t len)
{
	enum usb_ep_pingpong pp = usb.ep0_state.tx.pingpong;
	void *destbuf = usb.ep0_buf[pp];

	if (len > EP0_BUFSIZE)
		return (-1);
	memcpy(destbuf, buf, len);

	return (usb_tx(destbuf, len, len, NULL, NULL));
}


static void
usb_rx_queue_next(struct usbd_ep_pipe_state_t *s)
{
	struct USB_BD_t *bd = usb_get_bd(0, USB_EP_RX, s->pingpong);
	size_t thislen = s->transfer_size;

	if (thislen > s->ep_maxsize)
		thislen = s->ep_maxsize;

	bd->addr = s->data_buf + s->pos;

	/* XXX fairly inefficient assignment */
	bd->bd = ((struct USB_BD_t) {
			.bc = thislen,
			.dts = 1,
			.data01 = s->data01,
			.own = 1
		}).bd;
}

/**
 * Returns: 0 when this is was the last transfer, 1 if there is still
 * more to go.
 */
/* Defaults to EP0 for now */
/* XXX pass usb_stat to validate pingpong */
static int
usb_rx_next(void)
{
	struct usbd_ep_pipe_state_t *s = &usb.ep0_state.rx;

	/**
	 * Us being here means the previous transfer just completed
	 * successfully.  That means the host just toggled its data
	 * sync bit, and so do we.
	 */
	s->data01 ^= 1;

	struct USB_BD_t *bd = usb_get_bd(0, USB_EP_RX, s->pingpong);
	size_t thislen = bd->bc;

	s->transfer_size -= thislen;
	s->pos += thislen;

	/**
	 * We're done with this buffer now.  Switch the pingpong now
	 * before we might have to receive the next piece of data.
	 */
	s->pingpong ^= 1;

	/**
	 * If this is a short transfer, or we received what we
	 * expected, we're done.
	 */
	if (thislen < s->ep_maxsize || s->transfer_size == 0) {
		if (s->callback)
			s->callback(s->data_buf, s->pos, s->callback_data);
		return (0);
	}

	/**
	 * Otherwise we still need to receive more data.
	 */
	usb_rx_queue_next(s);

	return (1);
}

/**
 * Receive USB data (OUT device transaction)
 *
 * So far this function is specialized for EP 0 only.
 *
 * Returns: size to be received, or -1 on error.
 */
int
usb_rx(void *buf, size_t len, ep_callback_t cb, void *cb_data)
{
	struct usbd_ep_pipe_state_t *s = &usb.ep0_state.rx;

	s->data_buf = buf;
	s->transfer_size = len;
	s->pos = 0;
	s->callback = cb;
	s->callback_data = cb_data;

	usb_rx_queue_next(s);
	return (len);
}


/**
 *
 * Great resource: http://wiki.osdev.org/Universal_Serial_Bus
 *
 * Control Transfers
 * -----------------
 *
 * A control transfer consists of a SETUP transaction (1), zero or
 * more data transactions (IN or OUT) (2), and a final status
 * transaction (3).
 *
 * Token sequence (data toggle):
 * 1.  SETUP (0)
 * (2a. OUT (1) ... (toggling))
 * 3a. IN (1)
 *
 * or
 * 1.  SETUP (0)
 * 2b. IN (1) ... (toggling)
 * 3b. OUT (1)
 *
 * Report errors by STALLing the control EP after (1) or (2), so that
 * (3) will STALL.  Seems we need to clear the STALL after that so
 * that the next SETUP can make it through.
 *
 *
 */

/**
 * The following code is not written defensively, but instead only
 * asserts values that are essential for correct execution.  It
 * accepts a superset of the protocol defined by the standard.  We do
 * this to save space.
 */

static int
usb_handle_control(struct usb_ctrl_req_t *req)
{
	uint16_t zero16 = 0;

	if (req->type != USB_CTRL_REQ_STD) {
		/* XXX pass on to higher levels */
		goto err;
	}

	/* Only STD requests here */
	switch (req->request) {
	case USB_CTRL_REQ_GET_STATUS:
		/**
		 * Because we don't support remote wakeup or
		 * self-powered operation, and we are specialized to
		 * only EP 0 so far, all GET_STATUS replies are just
		 * empty.
		 */
		return (usb_tx_cp(&zero16, sizeof(zero16)));

	case USB_CTRL_REQ_CLEAR_FEATURE:
	case USB_CTRL_REQ_SET_FEATURE:
		/**
		 * Nothing to do.  Maybe return STALLs on illegal
		 * accesses?
		 */
		break;

	case USB_CTRL_REQ_SET_ADDRESS:
		/**
		 * We must keep our previous address until the end of
		 * the status stage;  therefore we can't set the
		 * address right now.  Since this is a special case,
		 * the EP 0 handler will take care of this later on.
		 */
		usb.address = req->value & 0x7f;
		usb.state = USBD_STATE_SETTING_ADDRESS;
		break;

	case USB_CTRL_REQ_GET_DESCRIPTOR:
		/* XXX locate descriptor and usb_tx it */
		break;

	case USB_CTRL_REQ_GET_CONFIGURATION:
		return (usb_tx_cp(&usb.config, 1)); /* XXX implicit LE
						     * */

	case USB_CTRL_REQ_SET_CONFIGURATION:
		/* XXX check config */
		usb.config = req->value;
		usb.state = USBD_STATE_CONFIGURED;
		break;

	case USB_CTRL_REQ_GET_INTERFACE:
		/* We only support iface setting 0 */
		return (usb_tx_cp(&zero16, 1));

	case USB_CTRL_REQ_SET_INTERFACE:
		/* We don't support alternate iface settings */
		goto err;

	default:
		goto err;
	}

	return (0);

err:
	return (-1);
}

/* Only EP0 for now; clears all pending transfers. XXX invoke callbacks? */
static void
usb_clear_transfers(void)
{
	struct USB_BD_t *bd = usb.bdt;

	memset(bd, 0, sizeof(*bd) * 4);
}

static void
usb_setup_control(void)
{
	void *buf = usb.ep0_buf[usb.ep0_state.rx.pingpong];

	usb.ep0_state.rx.data01 = USB_DATA01_DATA0;
	usb.ep0_state.tx.data01 = USB_DATA01_DATA1;
	usb_rx(buf, EP0_BUFSIZE, NULL, NULL);
}

void
usb_handle_control_ep(struct USB_STAT_t stat)
{
	struct USB_BD_t *bd;
	struct usb_ctrl_req_t *req;
	int r;

	bd = usb_get_bd_stat(stat);

	switch (bd->tok_pid) {
	case USB_PID_SETUP:
		usb_clear_transfers();

		req = bd->addr;
		r = usb_handle_control(req);
		switch (r) {
		default:
			/* Data transfer outstanding */
			usb.ctrl_state = USBD_CTRL_STATE_DATA;
			break;

		case 0:
			usb.ctrl_state = USBD_CTRL_STATE_STATUS;
			usb_tx(NULL, 0, 0, NULL, NULL); /* empty status transfer */
			break;

		case -1:
			/* error */
			usb_ep_stall(0);
			usb_setup_control();
			break;
		}
		USB0_CTL.txd_suspend = 0;
		break;

	case USB_PID_IN:
		if (usb_tx_next())
			break;

		goto status_or_done;

	case USB_PID_OUT:
		if (usb_rx_next())
			break;

status_or_done:
		switch (usb.ctrl_state) {
		case USBD_CTRL_STATE_DATA:
			usb.ctrl_state = USBD_CTRL_STATE_STATUS;

			/* empty status transfer */
			switch (bd->tok_pid) {
			case USB_PID_IN:
				usb.ep0_state.rx.data01 = USB_DATA01_DATA1;
				usb_rx(NULL, 0, NULL, NULL);
				break;

			default:
				usb.ep0_state.tx.data01 = USB_DATA01_DATA1;
				usb_tx(NULL, 0, 0, NULL, NULL);
				break;
			}
			break;

		default:
			/* done with status */
			usb.ctrl_state = USBD_CTRL_STATE_IDLE;
			if (usb.state == USBD_STATE_SETTING_ADDRESS) {
				usb.state = USBD_STATE_ADDRESS;
				USB0_ADDR.addr = usb.address;
			}
			usb_setup_control();
			break;
		}
		break;
	}
}