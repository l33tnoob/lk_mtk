ifeq ($(strip $(MTK_DUMP_RELEASE_INFO)),yes) 

$(info DUMP_RELEASE_INFO start)
$(info $(shell date +%s))
dump_release_info_folder := $(strip $(PRODUCT_OUT))/releaseInfo
$(shell mkdir -p $(dump_release_info_folder))
$(foreach MTK_HOOK_MODULE_ID,$(MTK_HOOK_ALL_MODULE_ID), \
  $(eval RELEASE_INFO_DISPLAY := ) \
  $(eval MTK_HOOK_MODULE_ID := $$(strip $$(MTK_HOOK_MODULE_ID))) \
  $(eval dump_release_info_file := $$(strip $$(dump_release_info_folder))/$$(MTK_HOOK_MODULE_ID)) \
  $(eval my_register_name-hook := $$(MTK_HOOK_$$(MTK_HOOK_MODULE_ID)_my_register_name-hook)) \
  $(eval RELEASE_INFO_DISPLAY := MODULE_INSTALLED_FILES = $(ALL_MODULES.$(my_register_name-hook).INSTALLED)\\n) \
  $(eval RELEASE_INFO_DISPLAY := $(RELEASE_INFO_DISPLAY)MODULE_BUILT_FILES = $(ALL_MODULES.$(my_register_name-hook).BUILT)\\n) \
  $(foreach MTK_HOOK_RELEASE_CHECK_ITEM,$(MTK_HOOK_RELEASE_ALL_CHECK_ITEM), \
    $(eval RELEASE_INFO_DISPLAY := $(RELEASE_INFO_DISPLAY)$(MTK_HOOK_RELEASE_CHECK_ITEM) = $(MTK_HOOK_$(MTK_HOOK_MODULE_ID)_$(MTK_HOOK_RELEASE_CHECK_ITEM))\\n) \
  ) \
  $(shell echo -e $(RELEASE_INFO_DISPLAY)>$(dump_release_info_file)) \
)
$(info $(shell date +%s))
$(info DUMP_RELEASE_INFO end)

dump_release_info:
	@echo Done.
endif
