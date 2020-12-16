include $(TOPDIR)/rules.mk

PKG_NAME:=libiot-watson-c

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/ibm-watson-iot/iot-embeddedc.git
PKG_SOURCE_VERSION:=1bbc817cb38db683d1d6995a718ad0f576734557
CMAKE_INSTALL:=1

#PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/libiot-watson-c
	SECTION:=libs
	CATEGORY:=Libraries
	TITLE:= C library to build applications connected to IBM Watson
endef

define Package/libiot-watson-c/description
	Embedded C client for interacting with the IBM Watson Internet of Things Platform.
endef

define Build/Prepare
	$(PKG_UNPACK)
	$(call Build/Prepare/Default)
	$(PKG_BUILD_DIR)/setup.sh
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib $(1)/usr/include/libiot-watson-c
	$(CP) $(PKG_BUILD_DIR)/src/*.so* $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/lib/*.h $(1)/usr/include/libiot-watson-c
	$(CP) $(PKG_BUILD_DIR)/lib/mbedtls/*.h $(1)/usr/include/libiot-watson-c
	$(CP) $(PKG_BUILD_DIR)/src/*.h $(1)/usr/include/libiot-watson-c
endef

define Package/libiot-watson-c/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/src/*.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libiot-watson-c))