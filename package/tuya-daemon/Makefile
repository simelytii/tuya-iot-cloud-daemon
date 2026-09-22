include $(TOPDIR)/rules.mk

PKG_NAME:=tuya-daemon
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/tuya-daemon
  CATEGORY:=Network
  TITLE:=Tuya IoT monitoring daemon
  DEPENDS:=+tuya-iot-core-sdk +libubus +libubox +libblobmsg-json +libuci +cJSON
endef

define Package/tuya-daemon/description
  Daemon that gathers router telemetry (memory, uptime, network
  interfaces, CPU usage) via ubus and reports it to the Tuya IoT cloud.
  It exposes a small ubus control API and logs to syslog (logread).
endef

define Package/tuya-daemon/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tuya-daemon \
		$(1)/usr/bin/tuya-daemon

	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/etc/init.d/tuya-daemon.init \
		$(1)/etc/init.d/tuya-daemon

	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/etc/config/tuya-daemon.config \
		$(1)/etc/config/tuya-daemon
endef

$(eval $(call BuildPackage,tuya-daemon))