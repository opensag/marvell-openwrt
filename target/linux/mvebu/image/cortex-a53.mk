ifeq ($(SUBTARGET),cortexa53)

define Device/globalscale-espressobin
  KERNEL_NAME := Image
  KERNEL := kernel-bin
  DEVICE_TITLE := ESPRESSObin (Marvell Armada 3700 Community Board)
  DEVICE_PACKAGES := e2fsprogs ethtool mkf2fs kmod-fs-vfat kmod-usb2 kmod-usb3 kmod-usb-storage
  IMAGES := sdcard.img.gz sysupgrade-rootfs.tar.gz
  IMAGE/sdcard.img.gz := boot-scr | boot-img-ext4 | sdcard-img-ext4 | gzip | append-metadata
  IMAGE/sysupgrade-rootfs.tar.gz := sysupgrade-rootfs
  DEVICE_DTS := armada-3720-espressobin
  DTS_DIR := $(DTS_DIR)/marvell
  SUPPORTED_DEVICES := globalscale,espressobin
endef
TARGET_DEVICES += globalscale-espressobin

define Device/xspeed-nmxx
  KERNEL_NAME := Image
  KERNEL := kernel-bin
  DEVICE_TITLE := NMXX (X-Speed Armada 3700 NMXX Board)
  DEVICE_PACKAGES := e2fsprogs ethtool mkf2fs kmod-fs-vfat kmod-usb2 kmod-usb3 kmod-usb-storage
  IMAGES := sysupgrade-rootfs.tar.gz
  IMAGE/sysupgrade-rootfs.tar.gz := sysupgrade-rootfs
  DEVICE_DTS := armada-3720-nmxx
  DTS_DIR := $(DTS_DIR)/marvell
  SUPPORTED_DEVICES := x-speed,nmxx
endef
TARGET_DEVICES += xspeed-nmxx

define Device/xspeed-rcxx
  KERNEL_NAME := Image
  KERNEL := kernel-bin
  DEVICE_TITLE := RCXX (X-Speed Armada 3700 RCXX Board)
  DEVICE_PACKAGES := e2fsprogs ethtool mkf2fs kmod-fs-vfat kmod-usb2 kmod-usb3 kmod-usb-storage
  IMAGES := sysupgrade-rootfs.tar.gz
  IMAGE/sysupgrade-rootfs.tar.gz := sysupgrade-rootfs
  DEVICE_DTS := armada-3720-rc01
  DTS_DIR := $(DTS_DIR)/marvell
  SUPPORTED_DEVICES := x-speed,rcxx
endef
TARGET_DEVICES += xspeed-rcxx

endif
