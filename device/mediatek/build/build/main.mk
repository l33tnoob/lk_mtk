#Variable
TOP_LOCATION             := $(shell pwd)
override JOBS            := $(shell grep processor /proc/cpuinfo | wc -l)
ANDROID_FOLDER           := ${TOP_LOCATION}/android
ANDROID_OUT              := ${ANDROID_FOLDER}/out
ANDROID_VENDOR           := ${ANDROID_FOLDER}/vendor
LOG_FOLDER               := $(ANDROID_OUT)/target/product/$(MTK_CONFIG)/log
KERNEL_CONFIG            ?= $(MTK_CONFIG)

#check source build/envsetup.sh
ifeq "${ANDROID_CONFIG}" ""
$(error Please source mediatek/mbldenv.sh your-project at first)
endif

#V[=PKGS]      show all logs, process,[default=0] close
V ?= 0
ifeq "$(V)" "0"
    Q := @
    ANDROID_QUIET :=
    NOECHO := @
else
    Q :=
    ANDROID_QUIET := showcommands
    NOECHO := 
endif

#Target
.PHONY: all build_check pl lk kernel android help

all: build_check pl lk kernel android image


build_check:
	$(Q)mkdir -p $(LOG_FOLDER)
	$(Q)((( python mediatek/build/tools/checkEnv.py -a ) 3>&1 1>&2 2>&3 | tee ${LOG_FOLDER}/env.err ; exit $${PIPESTATUS[0]} ) 2>&1 | tee ${LOG_FOLDER}/env.log ; exit $${PIPESTATUS[0]} ) ; exit "$$?"

pl: build_check
	$(Q)((( cd preloader && TARGET_PRODUCT=$(MTK_CONFIG) ./build.sh ) 3>&1 1>&2 2>&3 | tee ${LOG_FOLDER}/pl.err ; exit $${PIPESTATUS[0]} ) 2>&1 | tee ${LOG_FOLDER}/pl.log ; exit $${PIPESTATUS[0]} ) ; exit "$$?"


lk: build_check
	$(Q)((( cd lk && NOECHO=$(NOECHO) make -l ${JOBS} -j ${JOBS} $(MTK_CONFIG) ) 3>&1 1>&2 2>&3 | tee ${LOG_FOLDER}/lk.err ; exit $${PIPESTATUS[0]} ) 2>&1 | tee ${LOG_FOLDER}/lk.log ; exit $${PIPESTATUS[0]} ) ; exit "$$?"
 

kernel: build_check
	$(Q)mkdir -p kernel/${KERNELRELEASE}/out
	$(Q)((( cd kernel/${KERNELRELEASE} && make O=out $(KERNEL_CONFIG)_defconfig ) 3>&1 1>&2 2>&3 | tee ${LOG_FOLDER}/kernel.err ; exit $${PIPESTATUS[0]} ) 2>&1 | tee ${LOG_FOLDER}/kernel.log ; exit $${PIPESTATUS[0]} ) ; exit "$$?"
	$(Q)((( cd kernel/${KERNELRELEASE} && make V=$(V) -l ${JOBS} -j ${JOBS} O=out zImage modules ) 3>&1 1>&2 2>&3 | tee -a ${LOG_FOLDER}/kernel.err ; exit $${PIPESTATUS[0]} ) 2>&1 | tee -a ${LOG_FOLDER}/kernel.log ; exit $${PIPESTATUS[0]} ) ; exit "$$?"


android: build_check
	$(Q)((( cd android && make -l ${JOBS} -j ${JOBS} ${ANDROID_QUIET} ) 3>&1 1>&2 2>&3 | tee ${LOG_FOLDER}/android.err ; exit $${PIPESTATUS[0]} ) 2>&1 | tee ${LOG_FOLDER}/android.log ; exit $${PIPESTATUS[0]} ) ; exit "$$?"

image:
	$(Q)ln -nfs ../../../../../lk/build-$(MTK_CONFIG)/lk.bin $(ANDROID_OUT)/target/product/$(MTK_CONFIG)/lk.bin
	$(Q)ln -nfs ../../../../../lk/build-$(MTK_CONFIG)/logo.bin $(ANDROID_OUT)/target/product/$(MTK_CONFIG)/logo.bin
	$(Q)ln -nfs ../../../../../preloader/bin/preloader_$(MTK_CONFIG).bin  $(ANDROID_OUT)/target/product/$(MTK_CONFIG)/preloader_$(MTK_CONFIG).bin

#clean
.PHONY: clean clean_pl clean_lk clean_kernel clean_android

clean: clean_pl clean_lk clean_kernel clean_android

clean_pl:
	$(Q)rm -rf preloader/out* preloader/bin preloader/*.bin preloader/*.map preloader/*.csv preloader/*.txt preloader/platform/*/src/init/inc/preloader.h preloader/platform/*/src/security/inc/proj_cfg.h

clean_lk:
	$(Q)rm -rf lk/build-*	

clean_kernel:
	$(Q)rm -rf kernel/${KERNELRELEASE}/out/

clean_android:
	$(Q)rm -rf $(ANDROID_OUT)


help:
	@echo ""
	@echo "-------------------------------------------------------------------------------"
	@echo "Note:"
	@echo "-------------------------------------------------------------------------------"
	@echo "1. logs under android/out/target/product/<prj>/log"
	@echo ""
	@echo ""
	@echo "-------------------------------------------------------------------------------"
	@echo "Variable:"
	@echo "-------------------------------------------------------------------------------"
	@echo "V=1             Show all commands, ex. V=1 make"
	@echo "KERNEL_CONFIG   choose kernel config, ex. KERNEL_CONFIG=<prj> make"
	@echo ""
	@echo "-------------------------------------------------------------------------------"
	@echo "Common make targets:"
	@echo "-------------------------------------------------------------------------------"
	@echo "clean         Clean out folder"
	@echo "pl            Build Preloader"
	@echo "lk            Build LK"
	@echo "kernel        Build kernel"
	@echo "android       Build android" 
	@echo "build_check   Check the build tool and environment"
	@echo "-------------------------------------------------------------------------------"
	@echo ""
