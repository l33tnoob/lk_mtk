include device/htc/common/AndroidBoard.mk

ifneq ($(TARGET_BUILD_PREBUILT),true)
# XML-format config for HTC PnPMgr
include $(CLEAR_VARS)
XML_TARGET := $(TARGET_OUT_ETC)/pnp.xml
ENCODE_SCRIPT := device/htc/common/pnp/encode_xor.py
XMLCHECKER_SCRIPT := device/htc/common/pnp/xmlchecker.py
$(XML_TARGET): PRIVATE_SCRIPT := $(ENCODE_SCRIPT)
$(XML_TARGET): device/htc/$(TARGET_DEVICE)/pnp_raw.xml | $(ENCODE_SCRIPT)
	$(hide) python $(XMLCHECKER_SCRIPT) $<
	$(hide) mkdir -p $(TARGET_OUT_ETC)
	-$(hide) python $(PRIVATE_SCRIPT) $< $@
files: $(XML_TARGET)

# PnPMgr prebuilt binary
PNPMGR_TARGET := $(TARGET_OUT_EXECUTABLES)/pnpmgr
$(PNPMGR_TARGET): device/htc/common/pnp/prebuilts/pnpmgr
	$(hide) mkdir -p $(TARGET_OUT_EXECUTABLES)
	-$(hide) cp -f $< $@
files: $(PNPMGR_TARGET)

SET_FOTA_TARGET := $(TARGET_OUT_EXECUTABLES)/setFOTAfreq.sh
$(SET_FOTA_TARGET): device/htc/common/pnp/prebuilts/setFOTAfreq.sh
	$(hide) mkdir -p $(TARGET_OUT_EXECUTABLES)
	-$(hide) cp -f $< $@
files: $(SET_FOTA_TARGET)
endif
