#
# HTC Corporation Proprietary Rights Acknowledgment
#
# Copyright (C) 2008-2015 HTC Corporation
#
# All Rights Reserved.
#
# The information contained in this work is the exclusive property of HTC Corporation
# ("HTC").  Only the user who is legally authorized by HTC ("Authorized User") has
# right to employ this work within the scope of this statement.  Nevertheless, the
# Authorized User shall not use this work for any purpose other than the purpose
# agreed by HTC.  Any and all addition or modification to this work shall be
# unconditionally granted back to HTC and such addition or modification shall be
# solely owned by HTC.  No right is granted under this statement, including but not
# limited to, distribution, reproduction, and transmission, except as otherwise
# provided in this statement.  Any other usage of this work shall be subject to the
# further written consent of HTC.
#

SSD_TOOL_POLICY := HTC SSD Test Tool SEPolicy

SSD_TOOL_EXCLUDED := false

#
# the HTC_PROTECT_CONFIDENTIAL_FLAG is provided by build team, false in the project earlier stage, true in the final (CRC) stage
#
#$(info $(SSD_TOOL_POLICY): HTC_PROTECT_CONFIDENTIAL_FLAG=[$(HTC_PROTECT_CONFIDENTIAL_FLAG)])

ifeq ($(HTC_PROTECT_CONFIDENTIAL_FLAG),true)
  SSD_TOOL_EXCLUDED := true
endif

#
# the HTC_NO_TOOL_BUILD is similar to HTC_PROTECT_CONFIDENTIAL_FLAG but only be used on dashbaord, to simulate RC build
#
#$(info $(SSD_TOOL_POLICY): HTC_NO_TOOL_BUILD=[$(HTC_NO_TOOL_BUILD)])

ifeq ($(HTC_NO_TOOL_BUILD),true)
  SSD_TOOL_EXCLUDED := true
endif

#$(info $(SSD_TOOL_POLICY): MAKECMDGOALS=[$(MAKECMDGOALS)])

ifneq ($(filter htcdebugboot, $(MAKECMDGOALS)),)
  SSD_TOOL_EXCLUDED := false
endif

#$(info $(SSD_TOOL_POLICY): SSD_TOOL_EXCLUDED=[$(SSD_TOOL_EXCLUDED)])

ifeq ($(SSD_TOOL_EXCLUDED),false)

BOARD_SEPOLICY_DIRS += device/mediatek/common/ssd_tool

BOARD_SEPOLICY_UNION += \
        file_contexts \
        seapp_contexts \
        ssd_tool.te \
        su.te

endif # ifeq ($(SSD_TOOL_EXCLUDED),false)
