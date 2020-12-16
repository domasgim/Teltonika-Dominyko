include $(TOPDIR)/rules.mk

PKG_NAME:=luci_watson_web_prog
PKG_RELEASE:=1
PKG_VERSION:=1.0.0

include $(INCLUDE_DIR)/package.mk

define Package/luci_watson_web_prog
	CATEGORY:=Base system
	TITLE:=luci_watson_web_prog
	DEPENDS:=+libuci +libiot-watson-c
endef

define Package/luci_watson_web_prog/description
	This is an example app for IBM Watson IoT platform
endef

define Package/luci_watson_web_prog/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/etc/samples
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/luci_watson_web_prog $(1)/usr/bin
	$(INSTALL_BIN) ./files/watson.init $(1)/etc/init.d/watson
	$(INSTALL_BIN) ./files/IoTFoundation.pem $(1)/etc/IoTFoundation.pem
endef

$(eval $(call BuildPackage,luci_watson_web_prog))
