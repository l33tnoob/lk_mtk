
@HTC LK version

.global HTC_LK_HEADER
HTC_LK_HEADER:

	.ascii	"1.01"		@maximum 4 character, the first 4 byte is jump command
.if (HTC_BUILD_MODE == 0)
	.asciz	".0000"			@maximum 7 character
.elseif (HTC_BUILD_MODE == 1)
	.asciz	".1000"			@maximum 7 character
.else
	.asciz	".2000"			@maximum 7 character
.endif
	.balign	16

.if (HTC_BUILD_MODE == 0)
	.asciz	"SHIP"
.elseif (HTC_BUILD_MODE == 1)
	.asciz	"MFG"			@maximum 15 character
.else
	.asciz	"ENG"			@maximum 15 character
.endif
	.balign	16

	.include "htc_commit.inc"
