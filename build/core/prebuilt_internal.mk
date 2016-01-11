###########################################################
## Standard rules for copying files that are prebuilt
##
## Additional inputs from base_rules.make:
## None.
##
###########################################################

ifneq ($(LOCAL_PREBUILT_LIBS),)
$(error dont use LOCAL_PREBUILT_LIBS anymore LOCAL_PATH=$(LOCAL_PATH))
endif
ifneq ($(LOCAL_PREBUILT_EXECUTABLES),)
$(error dont use LOCAL_PREBUILT_EXECUTABLES anymore LOCAL_PATH=$(LOCAL_PATH))
endif
ifneq ($(LOCAL_PREBUILT_JAVA_LIBRARIES),)
$(error dont use LOCAL_PREBUILT_JAVA_LIBRARIES anymore LOCAL_PATH=$(LOCAL_PATH))
endif

# Check mechanism for preload application
# Add by U61
ifeq ($(LOCAL_MODULE), $(filter $(LOCAL_MODULE), $(HTC_USERDATA_APK)))
   # If application under data, don't do dex pre-opt
   LOCAL_DEX_PREOPT :=false
   LOCAL_MODULE_PATH := $(strip $(LOCAL_MODULE_PATH))
   ifeq ($(HTCBCC_BUILD_TO_PRELOAD),true)
      ifneq ($(LOCAL_MODULE_PATH) , $(PRODUCT_OUT)/preload)
          $(error $(LOCAL_MODULE) should set to new preload)
      endif
   else
      ifneq ($(LOCAL_MODULE_PATH) , $(TARGET_OUT_DATA)/preload)
         ifneq ($(LOCAL_MODULE_PATH) , $(TARGET_OUT_DATA)/app)
            $(error dont set LOCAL_MODULE_PATH LOCAL_PATH=$(LOCAL_PATH))
         endif
      endif
   endif
endif
# End Check mechanism for preload application

# Not much sense to check build prebuilts
LOCAL_DONT_CHECK_MODULE := true

my_32_64_bit_suffix := $(if $($(LOCAL_2ND_ARCH_VAR_PREFIX)$(my_prefix)IS_64_BIT),64,32)

ifdef LOCAL_PREBUILT_MODULE_FILE
  my_prebuilt_src_file := $(LOCAL_PREBUILT_MODULE_FILE)
else
  ifdef LOCAL_SRC_FILES_$($(my_prefix)$(LOCAL_2ND_ARCH_VAR_PREFIX)ARCH)
    my_prebuilt_src_file := $(LOCAL_PATH)/$(LOCAL_SRC_FILES_$($(my_prefix)$(LOCAL_2ND_ARCH_VAR_PREFIX)ARCH))
  else
    ifdef LOCAL_SRC_FILES_$(my_32_64_bit_suffix)
      my_prebuilt_src_file := $(LOCAL_PATH)/$(LOCAL_SRC_FILES_$(my_32_64_bit_suffix))
    else
      my_prebuilt_src_file := $(LOCAL_PATH)/$(LOCAL_SRC_FILES)
    endif
  endif
endif

ifeq (SHARED_LIBRARIES,$(LOCAL_MODULE_CLASS))
  # Put the built targets of all shared libraries in a common directory
  # to simplify the link line.
  OVERRIDE_BUILT_MODULE_PATH := $($(LOCAL_2ND_ARCH_VAR_PREFIX)$(my_prefix)OUT_INTERMEDIATE_LIBRARIES)
endif

ifneq ($(filter STATIC_LIBRARIES SHARED_LIBRARIES,$(LOCAL_MODULE_CLASS)),)
  prebuilt_module_is_a_library := true
else
  prebuilt_module_is_a_library :=
endif

# Don't install static libraries by default.
ifndef LOCAL_UNINSTALLABLE_MODULE
ifeq (STATIC_LIBRARIES,$(LOCAL_MODULE_CLASS))
  LOCAL_UNINSTALLABLE_MODULE := true
endif
endif

ifeq ($(LOCAL_MODULE_CLASS),APPS)
LOCAL_BUILT_MODULE_STEM := package.apk
LOCAL_INSTALLED_MODULE_STEM := $(LOCAL_MODULE).apk
endif

ifeq ($(LOCAL_STRIP_MODULE),true)
  ifdef LOCAL_IS_HOST_MODULE
    $(error Cannot strip host module LOCAL_PATH=$(LOCAL_PATH))
  endif
  ifeq ($(filter SHARED_LIBRARIES EXECUTABLES,$(LOCAL_MODULE_CLASS)),)
    $(error Can strip only shared libraries or executables LOCAL_PATH=$(LOCAL_PATH))
  endif
  ifneq ($(LOCAL_PREBUILT_STRIP_COMMENTS),)
    $(error Cannot strip scripts LOCAL_PATH=$(LOCAL_PATH))
  endif
  include $(BUILD_SYSTEM)/dynamic_binary.mk
  built_module := $(linked_module)
else  # LOCAL_STRIP_MODULE not true
  include $(BUILD_SYSTEM)/base_rules.mk
  built_module := $(LOCAL_BUILT_MODULE)

ifdef prebuilt_module_is_a_library
export_includes := $(intermediates)/export_includes
$(export_includes): PRIVATE_EXPORT_C_INCLUDE_DIRS := $(LOCAL_EXPORT_C_INCLUDE_DIRS)
$(export_includes) : $(LOCAL_MODULE_MAKEFILE)
	@echo Export includes file: $< -- $@
	$(hide) mkdir -p $(dir $@) && rm -f $@
ifdef LOCAL_EXPORT_C_INCLUDE_DIRS
	$(hide) for d in $(PRIVATE_EXPORT_C_INCLUDE_DIRS); do \
	        echo "-I $$d" >> $@; \
	        done
else
	$(hide) touch $@
endif

$(LOCAL_BUILT_MODULE) : | $(intermediates)/export_includes
endif  # prebuilt_module_is_a_library

# The real dependency will be added after all Android.mks are loaded and the install paths
# of the shared libraries are determined.
ifdef LOCAL_INSTALLED_MODULE
ifdef LOCAL_SHARED_LIBRARIES
$(LOCAL_2ND_ARCH_VAR_PREFIX)$(my_prefix)DEPENDENCIES_ON_SHARED_LIBRARIES += \
  $(my_register_name):$(LOCAL_INSTALLED_MODULE):$(subst $(space),$(comma),$(LOCAL_SHARED_LIBRARIES))

# We also need the LOCAL_BUILT_MODULE dependency,
# since we use -rpath-link which points to the built module's path.
built_shared_libraries := \
    $(addprefix $($(LOCAL_2ND_ARCH_VAR_PREFIX)$(my_prefix)OUT_INTERMEDIATE_LIBRARIES)/, \
    $(addsuffix $($(my_prefix)SHLIB_SUFFIX), \
        $(LOCAL_SHARED_LIBRARIES)))
$(LOCAL_BUILT_MODULE) : $(built_shared_libraries)
endif
endif

endif  # LOCAL_STRIP_MODULE not true

ifeq ($(LOCAL_MODULE_CLASS),APPS)
PACKAGES.$(LOCAL_MODULE).OVERRIDES := $(strip $(LOCAL_OVERRIDES_PACKAGES))

# Select dpi-specific source
ifdef LOCAL_DPI_VARIANTS
my_dpi := $(firstword $(filter $(LOCAL_DPI_VARIANTS),$(PRODUCT_AAPT_PREF_CONFIG) $(PRODUCT_AAPT_PREBUILT_DPI)))
ifdef my_dpi
ifdef LOCAL_DPI_FILE_STEM
my_prebuilt_dpi_file_stem := $(LOCAL_DPI_FILE_STEM)
else
my_prebuilt_dpi_file_stem := $(LOCAL_MODULE)_%.apk
endif
my_prebuilt_src_file := $(dir $(my_prebuilt_src_file))$(subst %,$(my_dpi),$(my_prebuilt_dpi_file_stem))
endif  # my_dpi
endif  # LOCAL_DPI_VARIANTS

rs_compatibility_jni_libs :=
include $(BUILD_SYSTEM)/install_jni_libs.mk

ifeq ($(LOCAL_CERTIFICATE),EXTERNAL)
  # The magic string "EXTERNAL" means this package will be signed with
  # the default dev key throughout the build process, but we expect
  # the final package to be signed with a different key.
  #
  # This can be used for packages where we don't have access to the
  # keys, but want the package to be predexopt'ed.
  LOCAL_CERTIFICATE := $(DEFAULT_SYSTEM_DEV_CERTIFICATE)
  PACKAGES.$(LOCAL_MODULE).EXTERNAL_KEY := 1

  $(built_module) : PRIVATE_PRIVATE_KEY := $(LOCAL_CERTIFICATE).pk8
  $(built_module) : PRIVATE_CERTIFICATE := $(LOCAL_CERTIFICATE).x509.pem
endif
ifeq ($(LOCAL_CERTIFICATE),)
  # It is now a build error to add a prebuilt .apk without
  # specifying a key for it.
  $(error No LOCAL_CERTIFICATE specified for prebuilt "$(my_prebuilt_src_file)")
else ifeq ($(LOCAL_CERTIFICATE),PRESIGNED)
  # The magic string "PRESIGNED" means this package is already checked
  # signed with its release key.
  #
  # By setting .CERTIFICATE but not .PRIVATE_KEY, this package will be
  # mentioned in apkcerts.txt (with certificate set to "PRESIGNED")
  # but the dexpreopt process will not try to re-sign the app.
  PACKAGES.$(LOCAL_MODULE).CERTIFICATE := PRESIGNED
  PACKAGES := $(PACKAGES) $(LOCAL_MODULE)
else
  # If this is not an absolute certificate, assign it to a generic one.
  ifeq ($(dir $(strip $(LOCAL_CERTIFICATE))),./)
      LOCAL_CERTIFICATE := $(dir $(DEFAULT_SYSTEM_DEV_CERTIFICATE))$(LOCAL_CERTIFICATE)
  endif

  PACKAGES.$(LOCAL_MODULE).PRIVATE_KEY := $(LOCAL_CERTIFICATE).pk8
  PACKAGES.$(LOCAL_MODULE).CERTIFICATE := $(LOCAL_CERTIFICATE).x509.pem
  PACKAGES := $(PACKAGES) $(LOCAL_MODULE)

  $(built_module) : PRIVATE_PRIVATE_KEY := $(LOCAL_CERTIFICATE).pk8
  $(built_module) : PRIVATE_CERTIFICATE := $(LOCAL_CERTIFICATE).x509.pem
endif

# Disable dex-preopt of prebuilts to save space, if requested.
ifeq ($(DONT_DEXPREOPT_PREBUILTS),true)
LOCAL_DEX_PREOPT := false
endif

# Disable dex-preopt of prebuilts to prevent some problem on 64BIT
ifeq (true,$(WITH_DEXPREOPT))
ifeq ($(findstring 64,$(TARGET_CPU_ABI)),64)
# If in our filter list, also don't pre-opt.
odex_disabled_apks := $(shell cat external/HtcBuildFlag/build_tools/filter.txt)
ifeq ($(findstring $(LOCAL_MODULE), $(odex_disabled_apks)), $(LOCAL_MODULE))
# LOCAL_MODULE in filter.txt
LOCAL_DEX_PREOPT :=false
else  # not in filter list
ifneq (false,$(LOCAL_DEX_PREOPT))
# Let 64bit's project always keep dex inside
LOCAL_DEX_PREOPT := nostripping
endif
endif
endif
endif

# A way to force every application do pre-opt, except under perload
ifeq (true,$(FORCE_ALL_APP_DEXPREOPT))
ifeq ($(findstring 64,$(TARGET_CPU_ABI)),64)
LOCAL_DEX_PREOPT := nostripping
else
# If not 64bit and it set LOCAL_DEX_PREOPT=False, let it keept it's dex...
ifeq (false,$(LOCAL_DEX_PREOPT))
LOCAL_DEX_PREOPT := false
endif
endif
endif

# If LOCAL_MODULE_PATH is under data/preload, don't do pre-opt
LOCAL_MODULE_PATH := $(strip $(LOCAL_MODULE_PATH))
ifeq ($(LOCAL_MODULE_PATH) , $(TARGET_OUT_DATA)/preload)
LOCAL_DEX_PREOPT :=false
endif
ifeq ($(LOCAL_MODULE_PATH) , $(PRODUCT_OUT)/preload)
LOCAL_DEX_PREOPT :=false
endif
# End Check mechanism for preload application

# Add By U61_Bill_Yang We won't do dex-preopt if it don't have dex inside apk
ifeq ($(LOCAL_MODULE_CLASS),APPS)
target_has_dex :=
target_has_dex := $(shell unzip -l $(my_prebuilt_src_file) | grep classes.dex)
ifeq ($(strip $(target_has_dex)),)
$(warning $(LOCAL_MODULE) don't have class.dex)
LOCAL_DEX_PREOPT := false
endif
target_has_dex :=
endif
# End By U61_Bill_Yang

# Add by U61_Bill_Yang, recover ode_filder_list mechanism...
ifeq ($(LOCAL_MODULE_CLASS),APPS)
EXISTED := $(shell test -e packages/apps/CustomizationsData/odexfilter/all.txt && echo $$?)
ifeq ($(EXISTED),0)
  odex_disabled_apks := $(shell cat packages/apps/CustomizationsData/odexfilter/all.txt)
  ifeq ($(findstring $(LOCAL_MODULE), $(odex_disabled_apks)), $(LOCAL_MODULE))
     LOCAL_DEX_PREOPT := false
  endif
endif
# Also support filter by RCMS Project.
EXISTED := $(shell test -e packages/apps/CustomizationsData/odexfilter/$(PRIVATE_RCMS_NAME).txt && echo $$?)
ifeq ($(EXISTED),0)
  odex_disabled_apks := $(shell cat packages/apps/CustomizationsData/odexfilter/$(PRIVATE_RCMS_NAME).txt)
  ifeq ($(findstring $(LOCAL_MODULE), $(odex_disabled_apks)), $(LOCAL_MODULE))
     LOCAL_DEX_PREOPT := false
  endif
endif
# End Also support filter by RCMS Project.
endif
# End by U61_Bill_Yang, recover ode_filder_list mechanism...

#######################################
# defines built_odex along with rule to install odex
include $(BUILD_SYSTEM)/dex_preopt_odex_install.mk
#######################################
# Sign and align non-presigned .apks.
$(built_module) : $(my_prebuilt_src_file) | $(ACP) $(ZIPALIGN) $(SIGNAPK_JAR)
	$(transform-prebuilt-to-target)
ifdef extracted_jni_libs
	$(hide) zip -d $@ 'lib/*.so'  # strip embedded JNI libraries.
endif
ifneq ($(LOCAL_CERTIFICATE),PRESIGNED)
	$(sign-package)
#[CCC_CERT] Nelson_Li start
ifeq ($(CCC_CERT),true)
	$(gen-ccc-cert-apk)
	sleep 2
	$(sign-package)
	$(align-package)
	$(gen-ccc-cert-appid)
	$(gen-ccc-cert-appid_to_out)
	$(gen-ccc-cert-apk)
	sleep 2
	$(sign-package)
	$(align-package)
endif
endif
ifdef LOCAL_DEX_PREOPT
ifneq (nostripping,$(LOCAL_DEX_PREOPT))
	$(call dexpreopt-remove-classes.dex,$@)
endif
endif
	$(align-package)

###############################
## Rule to build the odex file
ifdef LOCAL_DEX_PREOPT
$(built_odex) : $(my_prebuilt_src_file)
	$(call dexpreopt-one-file,$<,$@)
# ++[Debug][bowen_lai][2015/01/22][ART] add check for generating boot.oat
else
$(built_module): $(DEFAULT_DEX_PREOPT_BUILT_IMAGE_FILENAME)
ifdef TARGET_2ND_ARCH
$(built_module): $($(TARGET_2ND_ARCH_VAR_PREFIX)DEFAULT_DEX_PREOPT_BUILT_IMAGE_FILENAME)
endif
# --[Debug][bowen_lai][2015/01/22][ART] add check for generating boot.oat
endif

###############################
## Install split apks.
ifdef LOCAL_PACKAGE_SPLITS
# LOCAL_PACKAGE_SPLITS is a list of apks to be installed.
built_apk_splits := $(addprefix $(built_module_path)/,$(notdir $(LOCAL_PACKAGE_SPLITS)))
installed_apk_splits := $(addprefix $(my_module_path)/,$(notdir $(LOCAL_PACKAGE_SPLITS)))

# Rules to sign and zipalign the split apks.
my_src_dir := $(sort $(dir $(LOCAL_PACKAGE_SPLITS)))
ifneq (1,$(words $(my_src_dir)))
$(error You must put all the split source apks in the same folder: $(LOCAL_PACKAGE_SPLITS))
endif
my_src_dir := $(LOCAL_PATH)/$(my_src_dir)

$(built_apk_splits) : PRIVATE_PRIVATE_KEY := $(LOCAL_CERTIFICATE).pk8
$(built_apk_splits) : PRIVATE_CERTIFICATE := $(LOCAL_CERTIFICATE).x509.pem
$(built_apk_splits) : $(built_module_path)/%.apk : $(my_src_dir)/%.apk | $(ACP)
	$(copy-file-to-new-target)
	$(sign-package)
	$(align-package)

# Rules to install the split apks.
$(installed_apk_splits) : $(my_module_path)/%.apk : $(built_module_path)/%.apk | $(ACP)
	@echo "Install: $@"
	$(copy-file-to-new-target)

# Register the additional built and installed files.
ALL_MODULES.$(my_register_name).INSTALLED += $(installed_apk_splits)
ALL_MODULES.$(my_register_name).BUILT_INSTALLED += \
  $(foreach s,$(LOCAL_PACKAGE_SPLITS),$(built_module_path)/$(notdir $(s)):$(my_module_path)/$(notdir $(s)))

# Make sure to install the splits when you run "make <module_name>".
$(my_register_name): $(installed_apk_splits)

endif # LOCAL_PACKAGE_SPLITS

else # LOCAL_MODULE_CLASS != APPS
ifneq ($(LOCAL_PREBUILT_STRIP_COMMENTS),)
$(built_module) : $(my_prebuilt_src_file)
	$(transform-prebuilt-to-target-strip-comments)
else
$(built_module) : $(my_prebuilt_src_file) | $(ACP)
	$(transform-prebuilt-to-target)
ifneq ($(prebuilt_module_is_a_library),)
  ifneq ($(LOCAL_IS_HOST_MODULE),)
	$(transform-host-ranlib-copy-hack)
  else
	$(transform-ranlib-copy-hack)
  endif
endif
endif
endif # LOCAL_MODULE_CLASS != APPS

ifeq ($(LOCAL_IS_HOST_MODULE)$(LOCAL_MODULE_CLASS),JAVA_LIBRARIES)
# for target java libraries, the LOCAL_BUILT_MODULE is in a product-specific dir,
# while the deps should be in the common dir, so we make a copy in the common dir.
# For nonstatic library, $(common_javalib_jar) is the dependency file,
# while $(common_classes_jar) is used to link.
common_classes_jar := $(intermediates.COMMON)/classes.jar
common_javalib_jar := $(intermediates.COMMON)/javalib.jar

$(common_classes_jar) $(common_javalib_jar): PRIVATE_MODULE := $(LOCAL_MODULE)

ifneq ($(filter %.aar, $(my_prebuilt_src_file)),)
# This is .aar file, archive of classes.jar and Android resources.
my_src_jar := $(intermediates.COMMON)/aar/classes.jar

$(my_src_jar) : $(my_prebuilt_src_file)
	$(hide) rm -rf $(dir $@) && mkdir -p $(dir $@)
	$(hide) unzip -qo -d $(dir $@) $<
	# Make sure the extracted classes.jar has a new timestamp.
	$(hide) touch $@

else
# This is jar file.
my_src_jar := $(my_prebuilt_src_file)
endif
$(common_classes_jar) : $(my_src_jar) | $(ACP)
	$(transform-prebuilt-to-target)

$(common_javalib_jar) : $(common_classes_jar) | $(ACP)
	$(transform-prebuilt-to-target)

# make sure the classes.jar and javalib.jar are built before $(LOCAL_BUILT_MODULE)
$(built_module) : $(common_javalib_jar)
endif # TARGET JAVA_LIBRARIES

$(built_module) : $(LOCAL_ADDITIONAL_DEPENDENCIES)

my_prebuilt_src_file :=
