include $(TOPDIR)/rules.mk

PKG_NAME:=modemctl
PKG_RELEASE:=1
PKG_VERSION:=1.0.0

include $(INCLUDE_DIR)/package.mk

define Package/modemctl
	CATEGORY := Net
	TITLE:=Modemctl
	
endef

define Package/modemctl/description
	Command line tool to control modems
endef

define Package/modemctl/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/modemctl $(1)/usr/bin
endef

$(eval $(call BuildPackage,modemctl))
