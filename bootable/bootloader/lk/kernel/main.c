/*
 * Copyright (c) 2008 Travis Geiselbrecht
 *
 * Copyright (c) 2009, Code Aurora Forum. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include <compiler.h>
#include <debug.h>
#include <string.h>
#include <app.h>
#include <arch.h>
#include <platform.h>
#include <target.h>
#include <lib/heap.h>
#include <kernel/thread.h>
#include <kernel/timer.h>
#include <kernel/dpc.h>
#include <platform/mt_gpt.h>

#if _MAKE_HTC_LK
#include <htc_bootloader_log.h>
#endif
#ifdef MTK_LK_IRRX_SUPPORT
#include <platform/mtk_ir_lk_core.h>
#endif



extern void *__ctor_list;
extern void *__ctor_end;
extern int __bss_start;
extern int _end;

/* boot_time is calculated form kmain to kernel jump */
volatile unsigned int boot_time = 0;


static int bootstrap2(void *arg);

/* called from crt0.S */
void kmain(void) __NO_RETURN __EXTERNALLY_VISIBLE;
void kmain(void)
{
	boot_time = get_timer(0);
	zzytest_printf("kmain begin, boot_time=%d\n", boot_time);

	// get us into some sort of thread context
	thread_init_early();

	// early arch stuff
	arch_early_init();

	// do bootloader log init
	bldr_log_init(BOOT_DEBUG_LOG_BASE_PA, BOOT_DEBUG_LOG_SIZE);

	// do any super early platform initialization
	platform_early_init();

	zzytest_printf("zzytest, printf log end\n");
	dprintf(CRITICAL, "zzytest, after platform_early_init\n");

	// bring up the kernel heap
	heap_init();

	// initialize the threading system
	thread_init();

	// initialize the dpc system
	dpc_init();

	// initialize kernel timers
	timer_init();

	// create a thread to complete system initialization
	thread_resume(thread_create("bootstrap2", &bootstrap2, NULL, DEFAULT_PRIORITY, DEFAULT_STACK_SIZE));

	// enable interrupts
	exit_critical_section();

	// become the idle thread
	dprintf(CRITICAL, "zzytest, thread_become_idle\n");
	thread_become_idle();
}

int main(void);

extern const struct app_descriptor __apps_start;
static int bootstrap2(void *arg)
{
	dprintf(CRITICAL, "zzytest, bootstrap2\n");

	// initialize the rest of the platform
	platform_init();
	mt_boot_init(&__apps_start);

	return 0;
}
