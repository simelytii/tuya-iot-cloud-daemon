include $(TOPDIR)/rules.mk

PKG_NAME:=tuya-iot-core-sdk
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/tuya/tuya-iot-core-sdk.git
PKG_SOURCE_VERSION:=0208badefb2003d30e70b5b875395e3187c850d9

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

CMAKE_BINARY_SUBDIR:=build

define Package/tuya-iot-core-sdk
	CATEGORY:=Libraries
	TITLE:=Tuya IoT Core SDK
endef

define Package/tuya-iot-core-sdk/description
	Tuya IoT Core SDK for RutOS
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib $(1)/usr/include/tuya
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/interface/*.h $(1)/usr/include/tuya/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/include/tuyalink_core.h $(1)/usr/include/tuya/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/build/lib/*.so $(1)/usr/lib/
endef

define Build/Install
endef

define Package/tuya-iot-core-sdk/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/build/lib/*.so $(1)/usr/lib/
endef

$(eval $(call BuildPackage,tuya-iot-core-sdk))