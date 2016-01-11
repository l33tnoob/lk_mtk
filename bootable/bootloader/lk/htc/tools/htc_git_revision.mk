ifeq ($(BUILDDIR),)
$(error BUILDDIR is not defined!!!)
endif

HTC_COMMIT_INC=$(BUILDDIR)/htc_commit.inc
HTC_GIT_REVISION := $(shell htc/tools/get_git_revision)

$(info BUILDDIR: $(BUILDDIR))
$(info HTC_COMMIT_INC:   $(HTC_COMMIT_INC))
$(info HTC_GIT_REVISION: $(HTC_GIT_REVISION))

$(if $(shell [ "$$(echo -n $(HTC_GIT_REVISION) | wc -c)" -gt "15" ] && echo "FAILED"), \
$(error Git revision size greater than 15) \
)

GENERATED += $(HTC_COMMIT_INC)
EXTRA_BUILDDEPS += $(HTC_COMMIT_INC)

$(HTC_COMMIT_INC):
	@$(MKDIR)
	@echo "    .asciz     \"$(HTC_GIT_REVISION)\"" > $@
	@echo "    .balign    16" >> $@
