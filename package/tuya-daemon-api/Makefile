include $(TOPDIR)/rules.mk

PKG_NAME:=tuya-daemon-api
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/tuya-daemon-api
  CATEGORY:=Network
  TITLE:=Tuya daemon API
  DEPENDS:=+tuya-daemon
endef

define Package/tuya-daemon-api/description
  RutOS API for configuring the Tuya daemon.
endef

define Build/Compile
endef

define Package/tuya-daemon-api/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/api/services
	$(INSTALL_DATA) ./files/usr/lib/lua/api/services/tuya_daemon.lua \
		$(1)/usr/lib/lua/api/services/tuya_daemon.lua

	$(INSTALL_DIR) $(1)/usr/share/rpcd/acl.d
	$(INSTALL_DATA) ./files/usr/share/rpcd/acl.d/tuya-daemon-acl.json \
		$(1)/usr/share/rpcd/acl.d/tuya-daemon-acl.json

	$(INSTALL_DIR) $(1)/usr/share/vuci/path.d
	$(INSTALL_DATA) ./files/usr/share/vuci/path.d/tuya-daemon-vuci-route.json \
		$(1)/usr/share/vuci/path.d/tuya-daemon-vuci-route.json
endef

$(eval $(call BuildPackage,tuya-daemon-api))