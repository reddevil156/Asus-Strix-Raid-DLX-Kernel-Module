# Asus Strix Raid DLX Kernel Module for control box

Here you can find a kernel module for the control box of the ASUS Soundcard Strix Raid DLX.
Since ASUS doesn't provide drivers for linux I tried to reverse engineer the windows driver and wrote a kernel module for it.

I tested it under Manjaro Kernel 5.4 and 5.5 and its working fine. I cannot say how it works under other distributions, but it should do.

This is my first attempt to build a kernel module from scratch. So if you find errors please feel free to clone the repository and get rid of them and/or modify some code.
My code is licenced under GPLv2.

I am limited of time so I cannot provide much support.

## 1. Introduction

The ASUS soundcard Strix Raid DLX comes with a control box which is connected externally to the soundcard. Thix box is intended to be placed on the desk and you can plug in your headphones easily.
With a button you can switch between headphones (connected to the box) and speaker (directly connected to the soundcard). You can change also the volume with a rotation of the button.

The soundcard uses a Asmedia USB controller chip where the soundchip and the control box are internally connected. 
On button press a microcontroller inside the box starts an interrupt on interface 4 of the usb chip with a special request.

Here comes the module into play which reads the usb interrupts and changes the output between soundcard and headphone and can set the leds per volume.
A userspace daemon has access to this module. It can read the volume and set it to the master channel of alsamixer. If you change the volume with another method (keyboard, volume control
or other) the daemon writes the new volume to the module so the correct leds will be set on the control box.

## 2. Prerequisits

You will need:

kernel-sources
base-devel (debian: build-essential) with make and gcc
alsalib (libasound)
dkms


## 3. Installation

```bash
sudo sh dkms-install.sh
make daemon
```

The daemon should get access to the /dev/strixdlx, so copy the rules file
```bash
sudo cp 90-strixdlx.rules to /etc/udev/udev.rules.d/
```

Since the usbhid driver catches always the device before this kernel module gets access to it, we need usbhid to block for this device.
If usbhid is compiled as module you can use a quirk to block it:
```bash
sudo cp usbhid.conf /etc/modprobe.d/
```
Otherwise you need to build a udev rule to block it.


The userspace daemon can be copied where you want. I use /usr/bin for it:
```bash
sudo cp strix-daemon /usr/bin
```

The daemon must be started as user.
```bash
mkdir -p ~/.config/systemd/usr/
cp strix-daemon.service ~/config/systemd/usr/
systemctl --user start daemon.service
```

## 4. Manual Installation

You can use 
```bash
make
```
to build only the kernel module without dkms.
You can try it by using insmod or use xz and copy it to your kernel folder.


## 5. Licence :





