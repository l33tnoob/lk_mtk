ifeq ($(strip $(TARGET_CPU_VARIANT)),cortex-a53)
	arch_variant_cflags := -mcpu=cortex-a53
else
	arch_variant_cflags :=
endif

