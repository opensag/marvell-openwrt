#
# Copyright (C) 2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ecmdemo
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

define Package/ecmdemo
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=ETL device auto dial ECM tool
  DEPENDS:=+libc \
  		   +kmod-usb-net +kmod-usb-net-cdc-ether \
  		   +kmod-usb-serial +kmod-usb-serial-option +kmod-usb-serial-wwan \
  		   +kmod-ppp +kmod-ppp-synctty
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS) -Wall" \
		LDFLAGS="$(TARGET_LDFLAGS)"
endef

define Package/ecmdemo/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ECM_DEMO $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ECM_DEMO_AUTO $(1)/usr/sbin/
	$(INSTALL_BIN) ./files/ecm_demo $(1)/etc/init.d/
endef

$(eval $(call BuildPackage,ecmdemo))
