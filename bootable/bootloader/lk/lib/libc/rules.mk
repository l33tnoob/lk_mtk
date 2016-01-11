LOCAL_DIR := $(GET_LOCAL_DIR)

OBJS += \
	$(LOCAL_DIR)/atoi.o \
	$(LOCAL_DIR)/ctype.o \
	$(LOCAL_DIR)/printf.o \
	$(LOCAL_DIR)/malloc.o \
	$(LOCAL_DIR)/rand.o \
	$(LOCAL_DIR)/eabi.o

ifeq ($(MAKE_HTC_LK), 1)
OBJS += \
	$(LOCAL_DIR)/stdlib/atexit.o \
	$(LOCAL_DIR)/stdlib/qsort.o \
	$(LOCAL_DIR)/stdlib/strtoimax.o \
	$(LOCAL_DIR)/stdlib/strtol.o \
	$(LOCAL_DIR)/stdlib/strtoll.o \
	$(LOCAL_DIR)/stdlib/strtoul.o \
	$(LOCAL_DIR)/stdlib/strtoull.o \
	$(LOCAL_DIR)/stdlib/strtoumax.o \
	$(LOCAL_DIR)/sscanf.o \
	$(LOCAL_DIR)/itoa.o
endif

include $(LOCAL_DIR)/string/rules.mk

ifeq ($(WITH_CPP_SUPPORT),true)
OBJS += \
	$(LOCAL_DIR)/new.o \
	$(LOCAL_DIR)/atexit.o \
	$(LOCAL_DIR)/pure_virtual.o
endif

