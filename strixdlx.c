/*
 * Driver for the control box of Asus Strix Raid DLX Soundcard
 * 
 * 
 * Copyright (C) 2020 Tobias Wingerath
 * 
  *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 2.
 *
 * Inspired by the Dream Cheeky USB Missile Launcher driver
 * (ML-driver) by Matthias Vallentin <vallentin@icsi.berkeley.edu>
 * Much thanks, it helped me a lot :)
 * 
 * 
 * 
 * Some information about the card itsself:
 * 
 * The soundcard comes with a control box where the user can plugin the headset.
 * The main button can change the volume and switch between headphone and speaker.
 * 
 * The soundcard uses a asmedia usb chip where the box is connected to. The box
 * initializes a relay on the card to switch the output.
 * 
 * Therefore it uses usb interrupt messages and usb control messages.
 * 
 * ******	Receiving part	*******
 * 
 * It sends always two messages. The first one is always the same (16 bytes),
 * Its like a "hello I am here and I want to tell you something" message:
 *
 * 01 c5 00 00 01 01 0e 0e 00 00 00 00 00 00 00 00
 * 
 * The second message tells what the user had pressed at the box and what the driver
 * should do. The first, second and forth byte are important!
 * It can be one of the following:
 * 
 * 1. Main button is pressed, sound should be switched to the other output
 * 
 * 05 03 00 01 01 01 0e 0e 00 00 00 00 00 00 00 00
 * 
 * 2. Sonic Button is pressed (mainly for windows)
 * 
 * 05 02 00 01 01 00 0e 0e 00 00 00 00 00 00 00 00
 * 
 * 3. Decrease Volume
 * 
 * 05 06 00 01 01 04 0e 0e 00 00 00 00 00 00 00 00
 * 
 * 4. Increase Volume
 * 
 * 05 05 00 01 01 03 01 0e 00 00 00 00 00 00 00 00
 * 
 * 5. Box is not initialized
 * 
 * 05 04 00 01 01 03 01 0e 00 00 00 00 00 00 00 00
 * 
 * 
 * When the control box receives an corect usb message it responds with and waits for the
 * next interaction by the user:
 * 
 * 01 c5 00 00 01 01 0e 0e 00 00 00 00 00 00 00 00
 * 05 05 00 03 01 01 01 0e 00 00 00 00 00 00 00 00
 * 
 * ******	Sending part	******
 * For switching the output, it needs an usb control message with the following
 * parameters:
 * 
 * bmRequestType :0x21
 * bRequest: 0x01
 * wValue: 0x0800
 * wIndex: 0x0700
 * wLength: 2
 * 
 * The data block contains two bytes:
 * Switch from headphone to speaker:
 * {0x01, 0x03}
 * Switch from speaker to headphone:
 * {0x02, 0x03}
 * 
 * To control the leds on the panel the following parameters for the usb control message
 * are used:
 * 
 * bmRequestType :0x21
 * bRequest: 0x09
 * wValue: 0x0200
 * wIndex: 0x0004
 * wLength: 16
 * 
 * The data block contains 16 bytes. It looks like:
 * 
 * 09 c5 27 00 04 03 02 ff 1f 00 00 00 00 00 00 00
 * 
 * Byte 7 is used for the speaker/headphone led (0x02 = headphone, 0x08 = speaker)
 * Bytes 3, 8 and 9 represent the state of the volume control, how many leds are on.
 * 
 * Full table for headphone (from zero led to 13)
 * 
 * 09 c5 09 00 04 03 02 00 00 00 00 00 00 00 00 00	- headphone led on, zero volume leds
 * 09 c5 0a 00 04 03 02 01 00 00 00 00 00 00 00 00
 * 09 c5 0c 00 04 03 02 03 00 00 00 00 00 00 00 00
 * 09 c5 10 00 04 03 02 07 00 00 00 00 00 00 00 00
 * 09 c5 18 00 04 03 02 0f 00 00 00 00 00 00 00 00
 * 09 c5 28 00 04 03 02 1f 00 00 00 00 00 00 00 00
 * 09 c5 48 00 04 03 02 3f 00 00 00 00 00 00 00 00
 * 09 c5 88 00 04 03 02 7f 00 00 00 00 00 00 00 00
 * 09 c5 08 00 04 03 02 ff 00 00 00 00 00 00 00 00
 * 09 c5 09 00 04 03 02 ff 01 00 00 00 00 00 00 00
 * 09 c5 0b 00 04 03 02 ff 03 00 00 00 00 00 00 00
 * 09 c5 0f 00 04 03 02 ff 07 00 00 00 00 00 00 00
 * 09 c5 17 00 04 03 02 ff 0f 00 00 00 00 00 00 00
 * 09 c5 27 00 04 03 02 ff 1f 00 00 00 00 00 00 00	- headphone led on, all 13 volume leds on
 * 
 * Full table for speaker (from zero led to 13):
 * 
 * 09 c5 0f 00 04 03 08 00 00 00 00 00 00 00 00 00	- speaker led on, zero volume leds
 * 09 c5 10 00 04 03 08 01 00 00 00 00 00 00 00 00
 * 09 c5 12 00 04 03 08 03 00 00 00 00 00 00 00 00
 * 09 c5 16 00 04 03 08 07 00 00 00 00 00 00 00 00
 * 09 c5 1e 00 04 03 08 0f 00 00 00 00 00 00 00 00
 * 09 c5 2e 00 04 03 08 1f 00 00 00 00 00 00 00 00
 * 09 c5 4e 00 04 03 08 3f 00 00 00 00 00 00 00 00
 * 09 c5 8e 00 04 03 08 7f 00 00 00 00 00 00 00 00
 * 09 c5 0e 00 04 03 08 ff 00 00 00 00 00 00 00 00
 * 09 c5 0f 00 04 03 08 ff 01 00 00 00 00 00 00 00
 * 09 c5 11 00 04 03 08 ff 03 00 00 00 00 00 00 00
 * 09 c5 15 00 04 03 08 ff 07 00 00 00 00 00 00 00
 * 09 c5 1d 00 04 03 08 ff 0f 00 00 00 00 00 00 00
 * 09 c5 2d 00 04 03 08 ff 1f 00 00 00 00 00 00 00	- speaker led on, all 13 volume leds on
 * 
 * 
 * 
 */

#include <linux/module.h>
#include <linux/init.h>

#include <linux/slab.h>			/* kmalloc() */
#include <linux/usb.h>			/* USB stuff */
#include <linux/mutex.h>		/* mutexes */
#include <linux/ioctl.h>

#include <linux/uaccess.h>		/* copy_*_user */
#include <linux/poll.h>			/* polling */
#include <linux/wait.h>			/* wait queue */


#define DEBUG_LEVEL_DEBUG		0x1F
#define DEBUG_LEVEL_INFO		0x0F
#define DEBUG_LEVEL_WARN		0x07
#define DEBUG_LEVEL_ERROR		0x03
#define DEBUG_LEVEL_CRITICAL	0x01

/*
 * kernel messages
 */
#define DBG_DEBUG(fmt, args...) \
if ((debug_level & DEBUG_LEVEL_DEBUG) == DEBUG_LEVEL_DEBUG) \
	printk( KERN_DEBUG "[debug] %s(%d): " fmt "\n", \
			__FUNCTION__, __LINE__, ## args)
#define DBG_INFO(fmt, args...) \
if ((debug_level & DEBUG_LEVEL_INFO) == DEBUG_LEVEL_INFO) \
	printk( KERN_DEBUG "[info]  %s(%d): " fmt "\n", \
			__FUNCTION__, __LINE__, ## args)
#define DBG_WARN(fmt, args...) \
if ((debug_level & DEBUG_LEVEL_WARN) == DEBUG_LEVEL_WARN) \
	printk( KERN_DEBUG "[warn]  %s(%d): " fmt "\n", \
			__FUNCTION__, __LINE__, ## args)
#define DBG_ERR(fmt, args...) \
if ((debug_level & DEBUG_LEVEL_ERROR) == DEBUG_LEVEL_ERROR) \
	printk( KERN_DEBUG "[err]   %s(%d): " fmt "\n", \
			__FUNCTION__, __LINE__, ## args)
#define DBG_CRIT(fmt, args...) \
if ((debug_level & DEBUG_LEVEL_CRITICAL) == DEBUG_LEVEL_CRITICAL) \
	printk( KERN_DEBUG "[crit]  %s(%d): " fmt "\n", \
			__FUNCTION__, __LINE__, ## args)


/*
 * Asus STRIX Raid DLX Device ID
 */
#define STRIXDLX_VENDOR_ID	0x0B05
#define STRIXDLX_PRODUCT_ID	0x180C

/*
 * values for the urb control message for switching the relay between headphone 
 * and speaker
 */
#define STRIXDLX_CTRL_BUFFER_SIZE 	2
#define STRIXDLX_CTRL_REQUEST_TYPE	0x21
#define STRIXDLX_CTRL_REQUEST		0x01
#define STRIXDLX_CTRL_VALUE		0x0800
#define STRIXDLX_CTRL_INDEX		0x0700

/*
 * values for the urb control message for setting the sound leds
 */
#define STRIXDLX_CTRL_VOLUME_BUFFER_SIZE 	16
#define STRIXDLX_CTRL_VOLUME_REQUEST_TYPE	0x21
#define STRIXDLX_CTRL_VOLUME_REQUEST	0x09
#define STRIXDLX_CTRL_VOLUME_VALUE		0x0200
#define STRIXDLX_CTRL_VOLUME_INDEX		0x0004

/*
 * urb data array for setting the volume to max (all led on)
 */
const u8 STRIXDLX_VOLUME_SPEAKER[] = {0x09, 0xc5, 0x2d, 0x00, 0x04, 0x03, 0x08, 0xff, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
const u8 STRIXDLX_VOLUME_HEADPHONE[] = {0x09, 0xc5, 0x27, 0x00, 0x04, 0x03, 0x02, 0xff, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

/*
 * urb data array for switching relay between speaker and headphone
 */
const u8 STRIXDLX_DATA_SPEAKER[] = {0x01, 0x03 };
const u8 STRIXDLX_DATA_HEADPHONE[] = {0x02, 0x03 };



#define STRIXDLX_MINOR_BASE	0


/*
 * structure to hold all data
 */
struct strixdlx_usb{
	struct usb_device 	*udev;
	struct usb_interface 	*interface;
	unsigned char		minor;
	
	int				open_count;     /* count how often a program is connected */
	struct 			semaphore sem;	/* Locks this structure */
	spinlock_t		ctrl_spinlock;	/* lock for ctrl_volume_buffer  */
	spinlock_t		volume_spinlock;

	char				*int_in_buffer;
	struct usb_endpoint_descriptor  *int_in_endpoint;
	struct urb 			*int_in_urb;
	int				int_in_running;

	char			*ctrl_buffer; /* 2 byte buffer for switch control message */
	struct urb		*ctrl_urb;	  /* ctrl urb for relay control */	
	struct usb_ctrlrequest  *ctrl_dr;     /* Setup packet information for control message*/
	
	char			*ctrl_volume_buffer; /* 16 byte buffer for volume control message */
	struct urb		*ctrl_volume_urb;	  /* ctrl urb for volume control */	
	struct usb_ctrlrequest  *ctrl_volume_dr;     /* Setup packet information for volume message*/

	int 			box_int_registered; /* contains if box control box has send an interrupt */
	int 			control_setting; /* switch status: speaker = 0, headphone = 1 */

	char			readbuf[16];	/* read buffer for messages from userspace program */
	size_t			readbuflen;		/* buffer length of readbuf */

	int				volume_speaker; /* volume of speaker: 0-100 */
	int				volume_headphone; /* volume of headphone: 0 -100 */

	
};

/*
 * waiting queue for userspace program
 */
static wait_queue_head_t waitqueue;

/*
 * driver id table
 */
static struct usb_device_id strixdlx_table[] =
{
    { USB_DEVICE(STRIXDLX_VENDOR_ID, STRIXDLX_PRODUCT_ID) },
    {}, /* Terminating entry */
};
MODULE_DEVICE_TABLE (usb, strixdlx_table);

/*
 * internal debug
 */
static int debug_level = DEBUG_LEVEL_INFO;
static int debug_trace = 0;
module_param(debug_level, int, S_IRUGO | S_IWUSR);
module_param(debug_trace, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(debug_level, "debug level (bitmask)");
MODULE_PARM_DESC(debug_trace, "enable function tracing");

/* Prevent races between open() and disconnect */
static DEFINE_MUTEX(disconnect_mutex);
/*
 * stuct usb driver
 */
static struct usb_driver strixdlx_driver;

/*
 *	printout for urb data
 */
static inline void strixdlx_debug_data(const char *function, int size,
		const unsigned char *data)
{
	int i;

	if ((debug_level & DEBUG_LEVEL_DEBUG) == DEBUG_LEVEL_DEBUG) {
		printk(KERN_DEBUG "[debug] %s: length = %d, data = ",
		       function, size);
		for (i = 0; i < size; ++i)
			printk("%.2x ", data[i]);
		printk("\n");
	}
}

/*
 * Fills out the ctrl_volume_buffer for urb volume switching
 * strixdlx_usb *dev:  struct holding all data
 * int control: 0 if speaker, 1 if headphone
 */
static void SetVolume(struct strixdlx_usb *dev, int control){

	int i;
	u8 buf_volume[16];

	if (control == 1 ) {
		// fill out the complete array
		for (i = 0; i<16; i++ ) {
        	buf_volume[i] = STRIXDLX_VOLUME_HEADPHONE[i];
		}
		// check volume and modify bytes 2, 7 and 9. All other bytes are already set correctly
		if (dev->volume_headphone == 0) {
			buf_volume[2] = 0x09;
			buf_volume[7] = 0x00;
			buf_volume[8] = 0x00;
		}
		else if (dev->volume_headphone > 0 && dev->volume_headphone < 8) {
			buf_volume[2] = 0x0a;
			buf_volume[7] = 0x01;
			buf_volume[8] = 0x00;
		}
		else if (dev->volume_headphone > 7 && dev->volume_headphone < 15) {
			buf_volume[2] = 0x0c;
			buf_volume[7] = 0x03;
			buf_volume[8] = 0x00;
		}
		else if (dev->volume_headphone > 14 && dev->volume_headphone < 22) {
			buf_volume[2] = 0x10;
			buf_volume[7] = 0x07;
			buf_volume[8] = 0x00;
		}	
		else if (dev->volume_headphone > 21 && dev->volume_headphone < 29) {
			buf_volume[2] = 0x18;
			buf_volume[7] = 0x0f;
			buf_volume[8] = 0x00;
		}
		else if (dev->volume_headphone > 28 && dev->volume_headphone < 36) {
			buf_volume[2] = 0x28;
			buf_volume[7] = 0x1f;
			buf_volume[8] = 0x00;
		}
		else if (dev->volume_headphone > 35 && dev->volume_headphone < 43) {
			buf_volume[2] = 0x48;
			buf_volume[7] = 0x3f;
			buf_volume[8] = 0x00;
		}
		else if (dev->volume_headphone > 42 && dev->volume_headphone < 51) {
			buf_volume[2] = 0x88;
			buf_volume[7] = 0x7f;
			buf_volume[8] = 0x00;
		}
		else if (dev->volume_headphone > 50 && dev->volume_headphone < 60) {
			buf_volume[2] = 0x08;
			buf_volume[7] = 0xff;
			buf_volume[8] = 0x00;
		}
		else if (dev->volume_headphone > 59 && dev->volume_headphone < 68) {
			buf_volume[2] = 0x09;
			buf_volume[7] = 0xff;
			buf_volume[8] = 0x01;
		}
		else if (dev->volume_headphone > 67 && dev->volume_headphone < 76) {
			buf_volume[2] = 0x0b;
			buf_volume[7] = 0xff;
			buf_volume[8] = 0x03;
		}
		else if (dev->volume_headphone > 75 && dev->volume_headphone < 84) {
			buf_volume[2] = 0x0f;
			buf_volume[7] = 0xff;
			buf_volume[8] = 0x07;
		}
		else if (dev->volume_headphone > 83 && dev->volume_headphone < 92) {
			buf_volume[2] = 0x17;
			buf_volume[7] = 0xff;
			buf_volume[8] = 0x0f;
		}
		//dev->volume > 91
		else {
			buf_volume[2] = 0x27;
			buf_volume[7] = 0xff;
			buf_volume[8] = 0x1f;
		}
		//lock ctrl_volume_buffer and copy date into it
		spin_lock(&dev->ctrl_spinlock);
		memcpy(dev->ctrl_volume_buffer, &buf_volume, 16);
		spin_unlock(&dev->ctrl_spinlock);


	}
	else {
		// fill out the complete array
		for (i = 0; i<16; i++ ) {
            buf_volume[i] = STRIXDLX_VOLUME_SPEAKER[i];
		}
		// check volume and modify bytes 2, 7 and 9. All other bytes are already set correctly
		if (dev->volume_speaker == 0) {
			buf_volume[2] = 0x0f;
			buf_volume[7] = 0x00;
			buf_volume[8] = 0x00;
		}
		else if (dev->volume_speaker > 0 && dev->volume_speaker < 8) {
			buf_volume[2] = 0x10;
			buf_volume[7] = 0x01;
			buf_volume[8] = 0x00;
		}
		else if (dev->volume_speaker > 7 && dev->volume_speaker < 15) {
			buf_volume[2] = 0x12;
			buf_volume[7] = 0x03;
			buf_volume[8] = 0x00;
		}
		else if (dev->volume_speaker > 14 && dev->volume_speaker < 22) {
			buf_volume[2] = 0x16;
			buf_volume[7] = 0x07;
			buf_volume[8] = 0x00;
		}	
		else if (dev->volume_speaker > 21 && dev->volume_speaker < 29) {
			buf_volume[2] = 0x1e;
			buf_volume[7] = 0x0f;
			buf_volume[8] = 0x00;
		}
		else if (dev->volume_speaker > 28 && dev->volume_speaker < 36) {
			buf_volume[2] = 0x2e;
			buf_volume[7] = 0x1f;
			buf_volume[8] = 0x00;
		}
		else if (dev->volume_speaker > 35 && dev->volume_speaker < 43) {
			buf_volume[2] = 0x4e;
			buf_volume[7] = 0x3f;
			buf_volume[8] = 0x00;
		}
		else if (dev->volume_speaker > 42 && dev->volume_speaker < 51) {
			buf_volume[2] = 0x8e;
			buf_volume[7] = 0x7f;
			buf_volume[8] = 0x00;
		}
		else if (dev->volume_speaker > 50 && dev->volume_speaker < 60) {
			buf_volume[2] = 0x0e;
			buf_volume[7] = 0xff;
			buf_volume[8] = 0x00;
		}
		else if (dev->volume_speaker > 59 && dev->volume_speaker < 68) {
			buf_volume[2] = 0x0f;
			buf_volume[7] = 0xff;
			buf_volume[8] = 0x01;
		}
		else if (dev->volume_speaker > 67 && dev->volume_speaker < 76) {
			buf_volume[2] = 0x11;
			buf_volume[7] = 0xff;
			buf_volume[8] = 0x03;
		}
		else if (dev->volume_speaker > 75 && dev->volume_speaker < 84) {
			buf_volume[2] = 0x05;
			buf_volume[7] = 0xff;
			buf_volume[8] = 0x07;
		}
		else if (dev->volume_speaker > 83 && dev->volume_speaker < 92) {
			buf_volume[2] = 0x1d;
			buf_volume[7] = 0xff;
			buf_volume[8] = 0x0f;
		}
		//dev->volume > 91
		else {
			buf_volume[2] = 0x2d;
			buf_volume[7] = 0xff;
			buf_volume[8] = 0x1f;
		}
		//lock ctrl_volume_buffer and copy date into it
		spin_lock(&dev->ctrl_spinlock);
		memcpy(dev->ctrl_volume_buffer, &buf_volume, 16);
		spin_unlock(&dev->ctrl_spinlock);
	}

	
}

/*
 * callback for volume control. Nothing to do here, control box should be fine
 */
static void strixdlx_ctrl_callback(struct urb *urb)
{
	DBG_DEBUG("strixdlx_ctrl_callback executed");
	
}

/*
 * interrupt callback for receiving messages
 */
static void strixdlx_int_in_callback(struct urb *urb)
{

	struct strixdlx_usb *dev = urb->context;
	int retval = 0;
	u8 buf[2];
	unsigned char *data;
	
	DBG_DEBUG("strixdlx_int_in_callback entered");
		
	if (urb->status) {
		//urb->status == -ENOENT ||
		if (
				urb->status == -ECONNRESET ||
				urb->status == -ESHUTDOWN) {
			DBG_ERR("urb status (%d)", urb->status);
			return;
		} else {
			DBG_ERR("non-zero urb status (%d)", urb->status);
			goto resubmit;
			
		}
	}

	data = urb->transfer_buffer;

	/*
	 * we got a message from the control box and need to analyse it 
	 */


	//DATA = 0x01 0xC5 ..... -> control box has send a message
	//it's like a "hello" message
	//we will will except this and set box_int_registered to 1
	if (data[0] == 0x01 && data[1] == 0xc5 && dev->box_int_registered == 0) {
		dev->box_int_registered = 1;
		DBG_DEBUG("Data = 0x01 0xC5 -> box_int_registered");
		goto resubmit;
	}

	// DATA = 0x05 ..... and we have a message before so we check the message
	if (data[0] == 0x05 && dev->box_int_registered == 1) {
		DBG_DEBUG("Data = 0x05 .... we check the rest of the message");

		//DATA = 0x05 0x05 XX 0x03
		//Control box accepts our message and is finished with work
		if (data[1] == 0x05 && data[3] == 0x03) {
			DBG_DEBUG("Data = 0x05 0x05 x 0x03: box finished");
			dev->box_int_registered = 0;
		}

		//DATA = 0x05 0x05 0xXX 0x01
		//control box tells us to increase volume
		if (data[1] == 0x05 && data[3] == 0x01) {
			DBG_DEBUG("Data = 0x05 0x05 0xXX 0x01: increase volume");
			//headphone setting
			if (dev->control_setting == 1) {
				
				//we increase in 3% steps. set it at max to 100
				if (dev->volume_headphone >= 97){
					spin_lock(&dev->volume_spinlock);
					dev->volume_headphone = 100;
					spin_unlock(&dev->volume_spinlock);
				} 
				else {
					spin_lock(&dev->volume_spinlock);
					dev->volume_headphone = dev->volume_headphone + 3;
					spin_unlock(&dev->volume_spinlock);
				}
				SetVolume(dev, 1);
			}
			//speaker setting
			else {
				//we increase in 3% steps. set it at max to 100
				if (dev->volume_speaker >= 97){
					spin_lock(&dev->volume_spinlock);
					dev->volume_speaker = 100;
					spin_unlock(&dev->volume_spinlock);

				} 
				else {
					spin_lock(&dev->volume_spinlock);
					dev->volume_speaker = dev->volume_speaker + 3;
					spin_unlock(&dev->volume_spinlock);
				}
				SetVolume(dev, 0);
			}
			
			//fill out urb and send it
			usb_fill_control_urb(dev->ctrl_volume_urb, dev->udev,
				usb_sndctrlpipe(dev->udev, 0),
				(unsigned char *)dev->ctrl_volume_dr,
				dev->ctrl_volume_buffer,
				STRIXDLX_CTRL_VOLUME_BUFFER_SIZE,
				strixdlx_ctrl_callback,
				dev);
			
			retval = usb_submit_urb(dev->ctrl_volume_urb, GFP_ATOMIC);

			if (retval < 0) {
				DBG_ERR("usb_control_msg volume failed (%d)", retval);
				goto resubmit;
			}

			//wake up the userspace program and send new volume
			if (dev->control_setting == 1) {
				dev->readbuflen = snprintf(dev->readbuf, sizeof(dev->readbuf), "%d", dev->volume_headphone);
			}
			else {
				dev->readbuflen = snprintf(dev->readbuf, sizeof(dev->readbuf), "%d", dev->volume_speaker);
			}
			wake_up(&waitqueue);

			//we got our message from the box, we wait till the next "hello" message
			dev->box_int_registered = 0;
		}

		//DATA = 0x05 0x06 0xXX 0x01
		//control box tells us to increase volume
		if (data[1] == 0x06 && data[3] == 0x01) {
			DBG_DEBUG("Data = 0x05 0x05 0xXX 0x01: decrease volume");

			//headphone
			if (dev->control_setting == 1) {
				
				//we decrease in 3% steps. We cannot got lower than 0
				if (dev->volume_headphone <= 3){
					spin_lock(&dev->volume_spinlock);
					dev->volume_headphone = 0;
					spin_unlock(&dev->volume_spinlock);
					
				} 
				else {
					spin_lock(&dev->volume_spinlock);
					dev->volume_headphone = dev->volume_headphone - 3;
					spin_unlock(&dev->volume_spinlock);
				}
				SetVolume(dev, 1);
			}
			//speaker
			else {
				//we decrease in 3% steps. We cannot got lower than 0
				if (dev->volume_speaker <= 3){
					spin_lock(&dev->volume_spinlock);
					dev->volume_speaker = 0;
					spin_unlock(&dev->volume_spinlock);
				} 
				else {
					spin_lock(&dev->volume_spinlock);
					dev->volume_speaker = dev->volume_speaker - 3;
					spin_unlock(&dev->volume_spinlock);
				}
				SetVolume(dev, 0);
			}

			//fill out urb control message for volume and send it
			usb_fill_control_urb(dev->ctrl_volume_urb, dev->udev,
				usb_sndctrlpipe(dev->udev, 0),
				(unsigned char *)dev->ctrl_volume_dr,
				dev->ctrl_volume_buffer,
				STRIXDLX_CTRL_VOLUME_BUFFER_SIZE,
				strixdlx_ctrl_callback,
				dev);
			
			retval = usb_submit_urb(dev->ctrl_volume_urb, GFP_ATOMIC);

			if (retval < 0) {
				DBG_ERR("usb_control_msg volume failed (%d)", retval);
				goto resubmit;
			}

			//wake up userspace program and send new volume
			if (dev->control_setting == 1) {
				dev->readbuflen = snprintf(dev->readbuf, sizeof(dev->readbuf), "%d", dev->volume_headphone);
			}
			else {
				dev->readbuflen = snprintf(dev->readbuf, sizeof(dev->readbuf), "%d", dev->volume_speaker);
			}
			wake_up(&waitqueue);

			//we got our message from the box, we wait till the next "hello" message
			dev->box_int_registered = 0;
		}


		//DATA = 0x05 0x04 ....
		//only happens if box is not initalisiert, could not happen cause we set the volume at probe()
		if (data[1] == 0x04) {
			DBG_DEBUG("Data = 0x05 0x04: control box not initialized");
		}


		//DATA = 0x05 0x03 ....
		//big button on control box is pressed, change output to either headphone or speaker
		if (data[1] == 0x03)  {
			DBG_DEBUG("Data = 0x05 0x03: change sound output to either speaker or headphone");

			//if setting is 1, then we are already on headphones and want to switch to speaker
			if (dev->control_setting == 1) {

				buf[0] = STRIXDLX_DATA_SPEAKER[0];
				buf[1] = STRIXDLX_DATA_SPEAKER[1];

				SetVolume(dev, 0);
			//if setting is 0, then we are on speaker and want to switch to headphone
			} else {

				buf[0] = STRIXDLX_DATA_HEADPHONE[0];
				buf[1] = STRIXDLX_DATA_HEADPHONE[1];

				SetVolume(dev, 1);

			}
			//copy buffer to urb control buffer
			memcpy(dev->ctrl_buffer, &buf, 2);

			//fill out urb for switching output
			usb_fill_control_urb(dev->ctrl_urb, dev->udev,
				usb_sndctrlpipe(dev->udev, 0),
				(unsigned char *)dev->ctrl_dr,
				dev->ctrl_buffer,
				STRIXDLX_CTRL_BUFFER_SIZE,
				strixdlx_ctrl_callback,
				dev);
			//submit ctrl switch urb
			retval = usb_submit_urb(dev->ctrl_urb, GFP_ATOMIC);

			if (retval < 0) {
				DBG_ERR("usb_control_msg failed (%d)", retval);
				goto resubmit;
			}
			
			//fill out urb for volume
			usb_fill_control_urb(dev->ctrl_volume_urb, dev->udev,
				usb_sndctrlpipe(dev->udev, 0),
				(unsigned char *)dev->ctrl_volume_dr,
				dev->ctrl_volume_buffer,
				STRIXDLX_CTRL_VOLUME_BUFFER_SIZE,
				strixdlx_ctrl_callback,
				dev);
			
			//submit volume urb
			retval = usb_submit_urb(dev->ctrl_volume_urb, GFP_ATOMIC);

			if (retval < 0) {
				DBG_ERR("usb_control_msg volume failed (%d)", retval);
				goto resubmit;
			}
			else {
				//relay is switched, now switch internal and tell the userspace the correct volume for this output
				if (dev->control_setting == 1) {
					dev->control_setting = 0;
					dev->readbuflen = snprintf(dev->readbuf, sizeof(dev->readbuf), "%d", dev->volume_speaker);
					wake_up(&waitqueue);
				} else {
					dev->control_setting = 1;
					dev->readbuflen = snprintf(dev->readbuf, sizeof(dev->readbuf), "%d", dev->volume_headphone);
					wake_up(&waitqueue);
				}
			}
			dev->box_int_registered = 0;		

		}
		//DATA = 0x05 0x02 ....
		//Sonic Button on the control box is pressed. Since we don't have such software we use it for something else
		//We set the soundvolume to 0 if bigger than 0, else to 100
		//I would say: a mute for the poor man
		if (data[1] == 0x02) {
			DBG_DEBUG("Data = 0x05 0x02: Sonic Button; We set the volume to 0 or 100");
			dev->box_int_registered = 0;

			//headphone
			if (dev->control_setting == 1) {
				
				if (dev->volume_headphone > 0) {
					spin_lock(&dev->volume_spinlock);
					dev->volume_headphone = 0;
					spin_unlock(&dev->volume_spinlock);
				}
				else {
					spin_lock(&dev->volume_spinlock);
					dev->volume_headphone = 100;
					spin_unlock(&dev->volume_spinlock);
				}
				SetVolume(dev, 1);
			}
			//speaker
			else {

				if (dev->volume_speaker > 0) {
					spin_lock(&dev->volume_spinlock);
					dev->volume_speaker = 0;
					spin_unlock(&dev->volume_spinlock);
				}
				else {
					spin_lock(&dev->volume_spinlock);
					dev->volume_speaker = 100;
					spin_unlock(&dev->volume_spinlock);
				}
				SetVolume(dev, 0);
			}

			//fill urb volume message
			usb_fill_control_urb(dev->ctrl_volume_urb, dev->udev,
			usb_sndctrlpipe(dev->udev, 0),
			(unsigned char *)dev->ctrl_volume_dr,
			dev->ctrl_volume_buffer,
			STRIXDLX_CTRL_VOLUME_BUFFER_SIZE,
			strixdlx_ctrl_callback,
			dev);
			//and submit urb
			retval = usb_submit_urb(dev->ctrl_volume_urb, GFP_ATOMIC);
			if (retval < 0) {
				DBG_ERR("usb_control_msg volume failed (%d)", retval);
				goto resubmit;
			}
			//inform userspace program about new volume
			if (dev->control_setting == 1) {
				dev->readbuflen = snprintf(dev->readbuf, sizeof(dev->readbuf), "%d", dev->volume_headphone);
			}
			else {
				dev->readbuflen = snprintf(dev->readbuf, sizeof(dev->readbuf), "%d", dev->volume_speaker);
			}
			wake_up(&waitqueue);
		}
	}



//resubmit urb so we get new messages from control box (if there are any)
resubmit:
	if (dev->int_in_running && dev->udev) {
		retval = usb_submit_urb(dev->int_in_urb, GFP_ATOMIC);
		if (retval) {
			DBG_ERR("resubmitting urb failed (%d)", retval);
			dev->int_in_running = 0;
		}
	}
}


/*
 * Userspace program uses this function to access module
 * 
 */
static int strixdlx_open(struct inode *inode, struct file *file)
{
    struct strixdlx_usb *dev = NULL;
	struct usb_interface *interface;
	int subminor;
	int retval = 0;

    DBG_INFO("Open device");
	subminor = iminor(inode);

    mutex_lock(&disconnect_mutex);

    interface = usb_find_interface(&strixdlx_driver, subminor);
    if (! interface) {
		DBG_ERR("can't find device for minor %d", subminor);
		retval = -ENODEV;
		goto exit;
	}

    dev = usb_get_intfdata(interface);
	if (! dev) {
		retval = -ENODEV;
		goto exit;
	}

    /* lock this device */
	if (down_interruptible (&dev->sem)) {
		DBG_ERR("sem down failed");
		retval = -ERESTARTSYS;
		goto exit;
	}

    /* Increment usage count */
	++dev->open_count;
	if (dev->open_count > 1)
		DBG_DEBUG("open_count = %d", dev->open_count);

	/* Save our object in the file's private structure. */
	file->private_data = dev;

	up(&dev->sem);

exit:
	mutex_unlock(&disconnect_mutex);
	return retval;
}


/*
 * userspace program uses this function to read the current volume
 * gets values between 0-100
 */
static ssize_t strixdlx_read(struct file *file, char __user *user_buf, size_t len, loff_t *off) {


	struct strixdlx_usb *dev;
	ssize_t ret;

	dev = file->private_data;
	//copy to userspace
	if (copy_to_user(user_buf, dev->readbuf, dev->readbuflen )) {

		ret = -EFAULT;
	} else {
		ret = dev->readbuflen;
	}
	//reset buf length
	dev->readbuflen = 0;
	return ret;

}

/*
 * userspace program uses this function to poll and waits until new data is ready
 * gets values between 0-100
 */
unsigned int strixdlx_poll(struct file *file, struct poll_table_struct *wait) {

	struct strixdlx_usb *dev;

	dev = file->private_data;

	//wait until new data is ready
	poll_wait(file, &waitqueue, wait);
	if (dev->readbuflen)
		return POLLIN;
	else
		return 0;
	
}

/*
 * userspace program uses this function to submit new volume
 * allowed are values between 0 and 100
 */
static ssize_t strixdlx_write(struct file *file, const char __user *user_buf, size_t
		count, loff_t *ppos)
{
	struct strixdlx_usb *dev;
	int retval = 0;
	bool policy;
    int cmd = 0;    

	dev = file->private_data;

	/* Lock this object. */
	if (down_interruptible(&dev->sem)) {
		retval = -ERESTARTSYS;
		goto exit;
	}

	/* Verify that the device wasn't unplugged. */
	if (! dev->udev) {
		retval = -ENODEV;
		DBG_ERR("No device or device unplugged (%d)", retval);
		goto unlock_exit;
	}

	/* Verify that we actually have some data to write. */
	if (count == 0)
		goto unlock_exit;

	/* We only accept maximum 3 byte writes. */
	if (count > 1)
		count = 1;

	// copy from user
	if (copy_from_user(&cmd, user_buf, count)) {
		retval = -EFAULT;
		goto unlock_exit;
	}

	//if values are between 0 to 100, everything is ok
	policy = (cmd >= 0 || cmd <= 100);

	if (!policy) {
		DBG_ERR("illegal command issued");
		retval = -EINVAL;
		goto unlock_exit;
	}

	//headphone active: set new volume
	if (dev->control_setting == 1) {
		spin_lock(&dev->volume_spinlock);
		dev->volume_headphone = cmd;
		spin_unlock(&dev->volume_spinlock);
	}
	//speaker active: set new volume
	else {
		spin_lock(&dev->volume_spinlock);
		dev->volume_speaker = cmd;
		spin_unlock(&dev->volume_spinlock);
	}

	SetVolume(dev, dev->control_setting);
	
	//fill volume urb (leds neeed to be set correctly)
	usb_fill_control_urb(dev->ctrl_volume_urb, dev->udev,
		usb_sndctrlpipe(dev->udev, 0),
		(unsigned char *)dev->ctrl_volume_dr,
		dev->ctrl_volume_buffer,
		STRIXDLX_CTRL_VOLUME_BUFFER_SIZE,
		strixdlx_ctrl_callback,
		dev);
	
	//submit volume urb
	retval = usb_submit_urb(dev->ctrl_volume_urb, GFP_KERNEL);

	if (retval < 0) {
		DBG_ERR("usb_control_msg failed (%d)", retval);
		goto unlock_exit;
	}

	retval = count;

unlock_exit:
	up(&dev->sem);

exit:
	return retval;
}


/*
 * abort all transfers
 */
static void strixdlx_abort_transfers(struct strixdlx_usb *dev)
{
	if (! dev) {
		DBG_ERR("dev is NULL");
		return;
	}

	if (! dev->udev) {
		DBG_ERR("udev is NULL");
		return;
	}

	if (dev->udev->state == USB_STATE_NOTATTACHED) {
		DBG_ERR("udev not attached");
		return;
	}

	/* Shutdown transfer */
	if (dev->int_in_running) {
		dev->int_in_running = 0;
		mb();
		if (dev->int_in_urb)
			usb_kill_urb(dev->int_in_urb);
	}

	if (dev->ctrl_urb)
		usb_kill_urb(dev->ctrl_urb);
}

/*
 * free all structures
 */
static inline void strixdlx_delete(struct strixdlx_usb *dev)
{
	//at first abort all transfers
	strixdlx_abort_transfers(dev);

	if (dev->int_in_urb)
		usb_free_urb(dev->int_in_urb);
	if (dev->ctrl_urb)
		usb_free_urb(dev->ctrl_urb);

	kfree(dev->int_in_buffer);
	kfree(dev->ctrl_buffer);
	kfree(dev->ctrl_dr);
	kfree(dev->ctrl_volume_buffer);
	kfree(dev->ctrl_volume_dr);
	kfree(dev);
}

/*
 * Release driver
 */
static int strixdlx_release(struct inode *inode, struct file *file)
{
	struct strixdlx_usb *dev = NULL;
	int retval = 0;

	DBG_INFO("Release strixdlx");
	dev = file->private_data;

	if (! dev) {
		DBG_ERR("dev is NULL");
		retval =  -ENODEV;
		goto exit;
	}

	/* Lock our device */
	if (down_interruptible(&dev->sem)) {
		retval = -ERESTARTSYS;
		goto exit;
	}

	if (dev->open_count <= 0) {
		DBG_ERR("device not opened");
		retval = -ENODEV;
		goto unlock_exit;
	}

	if (! dev->udev) {
		DBG_DEBUG("device unplugged before the file was released");
		up (&dev->sem);
		//delete & free structures
		strixdlx_delete(dev);
		goto exit;
	}

	if (dev->open_count > 1)
		DBG_DEBUG("open_count = %d", dev->open_count);

	--dev->open_count;

unlock_exit:
	up(&dev->sem);

exit:
	return retval;
}

/*
 * fops structure
 */
static struct file_operations strixdlx_fops = {
	.owner =	THIS_MODULE,
	.write =	strixdlx_write, 	/* write function for userspace; set volume */
	.open =		strixdlx_open, 		/* open device function for userspace */
	.release =	strixdlx_release, 	/* free device for userspace */
	.read = 	strixdlx_read, 		/* read function for userspace; get volume */
	.poll = 	strixdlx_poll, 		/* poll function for userspace */
};

/*
 * strixdlx class
 */
static struct usb_class_driver strixdlx_class = {
	.name = "strixdlx",
	.fops = &strixdlx_fops,
	.minor_base = STRIXDLX_MINOR_BASE,
};

/*
 * Probe function for usb driver
 */
static int strixdlx_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
	int retval = -ENODEV;
    struct usb_device *udev = interface_to_usbdev(interface);
	struct strixdlx_usb *dev = NULL;
	struct usb_host_interface *iface_desc;
	struct usb_endpoint_descriptor *endpoint;
	int i, int_end_size;
	u8 buf[2];				//buffer for relay
	u8 buf_volume[16];		//buffer for volume

    DBG_INFO("Probe strix dlx driver");

    if (interface->cur_altsetting->desc.bInterfaceNumber != 4) {
        goto exit;
    }

    if (! udev) {
		DBG_ERR("udev is NULL");
		goto exit;
	}

    dev = kzalloc(sizeof(struct strixdlx_usb), GFP_KERNEL);
    if (! dev) {
		DBG_ERR("cannot allocate memory for struct strixdlx_usb");
		retval = -ENOMEM;
		goto exit;
	}
    
    sema_init(&dev->sem, 1);
	spin_lock_init(&dev->ctrl_spinlock);

    dev->udev = udev;
	dev->interface = interface;
	iface_desc = interface->cur_altsetting;

	/* Set up interrupt endpoint information. */
	for (i = 0; i < iface_desc->desc.bNumEndpoints; ++i) {
		endpoint = &iface_desc->endpoint[i].desc;

		if (((endpoint->bEndpointAddress & USB_ENDPOINT_DIR_MASK)
		     == USB_DIR_IN)
		    && ((endpoint->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK)
		    	== USB_ENDPOINT_XFER_INT))
			dev->int_in_endpoint = endpoint;

	}

    if (! dev->int_in_endpoint) {
		DBG_ERR("could not find interrupt in endpoint");
		goto error;
	}

    int_end_size = le16_to_cpu(dev->int_in_endpoint->wMaxPacketSize);

    dev->int_in_buffer = kmalloc(int_end_size, GFP_KERNEL);
	if (! dev->int_in_buffer) {
		DBG_ERR("could not allocate int_in_buffer");
		retval = -ENOMEM;
		goto error;
	}
	/* allocate receiving urb */
	dev->int_in_urb = usb_alloc_urb(0, GFP_KERNEL);
	if (! dev->int_in_urb) {
		DBG_ERR("could not allocate int_in_urb");
		retval = -ENOMEM;
		goto error;
	}

	/* allocate control urb for switching between speaker and headphone */
	dev->ctrl_urb = usb_alloc_urb(0, GFP_KERNEL);
	if (! dev->ctrl_urb) {
		DBG_ERR("could not allocate ctrl_urb");
		retval = -ENOMEM;
		goto error;
	}

	/* allocate buffer for switching between speaker and headphone */
	dev->ctrl_buffer = kzalloc(STRIXDLX_CTRL_BUFFER_SIZE, GFP_KERNEL);
	if (! dev->ctrl_buffer) {
		DBG_ERR("could not allocate ctrl_buffer");
		retval = -ENOMEM;
		goto error;
	}

	/* allocate ctrlrequest for switching between speaker and headphone */
	dev->ctrl_dr = kmalloc(sizeof(struct usb_ctrlrequest), GFP_KERNEL);
	if (! dev->ctrl_dr) {
		DBG_ERR("could not allocate usb_ctrlrequest");
		retval = -ENOMEM;
		goto error;
	}

	/* allocate control urb for changing volume*/
	dev->ctrl_volume_urb = usb_alloc_urb(0, GFP_KERNEL);
	if (! dev->ctrl_volume_urb) {
		DBG_ERR("could not allocate ctrl_volume_urb");
		retval = -ENOMEM;
		goto error;
	}

	/* allocate buffer for changing volume*/
	dev->ctrl_volume_buffer = kzalloc(STRIXDLX_CTRL_VOLUME_BUFFER_SIZE, GFP_KERNEL);
	if (! dev->ctrl_volume_buffer) {
		DBG_ERR("could not allocate ctrl_volume_buffer");
		retval = -ENOMEM;
		goto error;
	}

	/* allocate ctrlrequest for changing volume*/
	dev->ctrl_volume_dr = kmalloc(sizeof(struct usb_ctrlrequest), GFP_KERNEL);
	if (! dev->ctrl_volume_dr) {
		DBG_ERR("could not allocate usb_ctrlrequest for volume");
		retval = -ENOMEM;
		goto error;
	}

	//fill buffer for initial relay setting to speaker
	buf[0] = STRIXDLX_DATA_SPEAKER[0];
	buf[1] = STRIXDLX_DATA_SPEAKER[1];

	//fill buffer for initial volume setting of speaker to 100%
	for (i = 0; i<16; i++ ) {
        buf_volume[i] = STRIXDLX_VOLUME_SPEAKER[i];
	}

	memcpy(dev->ctrl_buffer, &buf, 2);
	memcpy(dev->ctrl_volume_buffer, &buf_volume, 16);

	//control urb for switching to speaker
    dev->ctrl_dr->bRequestType = STRIXDLX_CTRL_REQUEST_TYPE;
	dev->ctrl_dr->bRequest = STRIXDLX_CTRL_REQUEST;
	dev->ctrl_dr->wValue = cpu_to_le16(STRIXDLX_CTRL_VALUE);
	dev->ctrl_dr->wIndex = cpu_to_le16(STRIXDLX_CTRL_INDEX);
	dev->ctrl_dr->wLength = cpu_to_le16(STRIXDLX_CTRL_BUFFER_SIZE);

	usb_fill_control_urb(dev->ctrl_urb, dev->udev,
			usb_sndctrlpipe(dev->udev, 0),
			(unsigned char *)dev->ctrl_dr,
			dev->ctrl_buffer,
			STRIXDLX_CTRL_BUFFER_SIZE,
			strixdlx_ctrl_callback,
			dev);

	//submit control urb
	retval = usb_submit_urb(dev->ctrl_urb, GFP_ATOMIC);
	if (retval < 0 ) {
		goto error;
	}

	//volume urb for setting sound to 100%
	dev->ctrl_volume_dr->bRequestType = STRIXDLX_CTRL_VOLUME_REQUEST_TYPE;
	dev->ctrl_volume_dr->bRequest = STRIXDLX_CTRL_VOLUME_REQUEST;
	dev->ctrl_volume_dr->wValue = cpu_to_le16(STRIXDLX_CTRL_VOLUME_VALUE);
	dev->ctrl_volume_dr->wIndex = cpu_to_le16(STRIXDLX_CTRL_VOLUME_INDEX);
	dev->ctrl_volume_dr->wLength = cpu_to_le16(STRIXDLX_CTRL_VOLUME_BUFFER_SIZE);

	usb_fill_control_urb(dev->ctrl_volume_urb, dev->udev,
		usb_sndctrlpipe(dev->udev, 0),
		(unsigned char *)dev->ctrl_volume_dr,
		dev->ctrl_volume_buffer,
		STRIXDLX_CTRL_VOLUME_BUFFER_SIZE,
		strixdlx_ctrl_callback,
		dev);
	
	//submit volume urb
	retval = usb_submit_urb(dev->ctrl_volume_urb, GFP_ATOMIC);
	if (retval < 0 ) {
		goto error;
	}

	//fill out receiving interrupt urb
	usb_fill_int_urb(dev->int_in_urb, dev->udev,
			usb_rcvintpipe(dev->udev,
				       dev->int_in_endpoint->bEndpointAddress),
			dev->int_in_buffer,
			le16_to_cpu(dev->int_in_endpoint->wMaxPacketSize),
			strixdlx_int_in_callback,
			dev,
			dev->int_in_endpoint->bInterval);

	//program is successful running
	dev->int_in_running = 1;
	//nothing yet from the control box received
	dev->box_int_registered = 0;
	//initial status is speaker
	dev->control_setting = 0;

	//set both volumes to 100% internally
	spin_lock(&dev->volume_spinlock);
	dev->volume_speaker = 100;
	dev->volume_headphone = 100;
	spin_unlock(&dev->volume_spinlock);
	
	
	//send receiving interrupt urb
	retval = usb_submit_urb(dev->int_in_urb, GFP_KERNEL);
	if (retval) {
		DBG_ERR("could not send int_in_urb");
		usb_set_intfdata(interface, NULL);
		goto error;
	}


    /* Save our data pointer in this interface device. */
	usb_set_intfdata(interface, dev);

    /* We can register the device now, as it is ready. */
	retval = usb_register_dev(interface, &strixdlx_class);
	if (retval) {
		DBG_ERR("not able to get a minor for this device.");
		usb_set_intfdata(interface, NULL);
		goto error;
	}

    dev->minor = interface->minor;

	//init waiting queue
	init_waitqueue_head(&waitqueue);

	//tell our userspace program the new volumes
	if (dev->control_setting == 1) {
		dev->readbuflen = snprintf(dev->readbuf, sizeof(dev->readbuf), "%d", dev->volume_headphone);
	}
	else {
		dev->readbuflen = snprintf(dev->readbuf, sizeof(dev->readbuf), "%d", dev->volume_speaker);
	}
	wake_up(&waitqueue);

	DBG_INFO("strixdlx_driver now attached to /dev/strixdlx");

exit:
    return retval;    

error:

    strixdlx_delete(dev);
    return retval;   
}

/*
 * Disconnect function when module is removed
 */
static void strixdlx_disconnect(struct usb_interface *interface)
{
    struct strixdlx_usb *dev;
	int minor;

	mutex_lock(&disconnect_mutex);

	dev = usb_get_intfdata(interface);
	usb_unlink_urb(dev->int_in_urb);
	usb_unlink_urb(dev->ctrl_urb);
	usb_set_intfdata(interface, NULL);
	
	down(&dev->sem);
	dev->int_in_running = 0;

    minor = dev->minor;

	/* Give back our minor. */
	usb_deregister_dev(interface, &strixdlx_class);

    /* If the device is not opened, then we clean up right now. */
	if (! dev->open_count) {
		up(&dev->sem);
		strixdlx_delete(dev);
	} else {
		dev->udev = NULL;
		up(&dev->sem);
	}

    mutex_unlock(&disconnect_mutex);

	DBG_INFO("strixdlx_dlx /dev/strixdlx now disconnected");
	//DBG_INFO("strixdlx_dlx /dev/strixdlx%d now disconnected",
	//		minor - STRIXDLX_MINOR_BASE);

}

/*
 * Suspend function
 * We clean up our urb requests then we are ready to sleep
 */
static int strixdlx_suspend(struct usb_interface *interface, pm_message_t message)
{

	struct strixdlx_usb *dev;
	mutex_lock(&disconnect_mutex);

	dev = usb_get_intfdata(interface);
	usb_unlink_urb(dev->int_in_urb);
	usb_unlink_urb(dev->ctrl_urb);

	mutex_unlock(&disconnect_mutex);

	DBG_INFO("strixdlx driver going to suspend");
	
	return 0;

}

/*
 * Resume function from sleep
 * We start again our receiving interrupt urb
 */
static int strixdlx_resume(struct usb_interface *interface)
{

	struct strixdlx_usb *dev;
	int retval = -ENODEV;

	DBG_INFO("strixdlx driver resume");

	dev = usb_get_intfdata(interface);
	if (!dev) {
		goto error;
	}
	
	//fill out the receiving interrupt urb
	usb_fill_int_urb(dev->int_in_urb, dev->udev,
			usb_rcvintpipe(dev->udev,
				       dev->int_in_endpoint->bEndpointAddress),
			dev->int_in_buffer,
			le16_to_cpu(dev->int_in_endpoint->wMaxPacketSize),
			strixdlx_int_in_callback,
			dev,
			dev->int_in_endpoint->bInterval);

	//we are again running
	dev->int_in_running = 1;
	dev->box_int_registered = 0;
	
	//submit urb
	retval = usb_submit_urb(dev->int_in_urb, GFP_KERNEL);
	if (retval) {
		DBG_ERR("could not send int_in_urb");
		usb_set_intfdata(interface, NULL);
		goto error;
	}

    return retval;    

error:
    strixdlx_delete(dev);
    return retval;   

}

/*
 * driver struct
 */
static struct usb_driver strixdlx_driver =
{
    .name = "strixdlx",
    .id_table = strixdlx_table,
    .probe = strixdlx_probe,
    .disconnect = strixdlx_disconnect,
	.suspend = strixdlx_suspend,
	.resume = strixdlx_resume,
};

/*
 * Init function
 */
static int __init strixdlx_init(void)
{
	int result;

	DBG_INFO("Register strixdlx driver");
	result = usb_register(&strixdlx_driver);
	if (result) {
		DBG_ERR("registering strixdlx driver failed");
	} else {
		DBG_INFO("driver strixdlx registered successfully");
	}
	return result;
}

/*
 * Exit function
 */
static void __exit strixdlx_exit(void)
{
    usb_deregister(&strixdlx_driver);
	DBG_INFO("driver strixdlx deregistered");
}

module_init(strixdlx_init);
module_exit(strixdlx_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tobias Wingerath");
MODULE_DESCRIPTION("USB Driver for Asus Strix Raid DLX Control Box");
MODULE_VERSION("1.0");