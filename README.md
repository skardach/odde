# Optical Disk Drive Eject (ODDE)

This driver was created for Fujitsu Esprimo Optical Disc Drive Eject button which happens to be completely separate from the slot optical drive on the Q910 Esprimo PC.

## Pre-requisites

1. Install kernel headers
2. (optional) Install dkms for automatic module rebuild on new kernel install

## Build & Installation

Get the source via `git clone` or downloading the ZIP package from github.

### Manual build

If you would like to test the module, you can build and insert it manually:

```bash
$ make -C src all
$ sudo insmod ./src/odde.ko
```

Next you can install the module and run depmod to make modprobe pick it up:

```bash
$ make -C src install
$ sudo depmod -a
```

### DKMS

There is a DKMS configuration file that allows for automatic rebuild on the new kernel install:

```bash
$ sudo cp -R . /usr/src/odde-1.0
$ sudo dkms add -m odde -v 1.0
```

Next to force building of the module for current kernel, do:

```bash
$ sudo dkms build -m odde -v 1.0
$ sudo dkms install -m odde -v 1.0
```

## Using the module

After installing the module in manual or DKMS way, you should be able to insert it with modprobe:

```bash
$ modprobe odde
```

The module should auto-load on system boot. If for some reason it doesn't add it to `/etc/modules` (or add a file in `/etc/modules-load.d` if your distro supports it).
