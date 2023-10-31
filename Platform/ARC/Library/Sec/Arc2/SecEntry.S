/** @file
  ARC SEC phase entry point.

  Copyright (c) 2023 Basemark Oy

  Author: Aliaksei Katovich @ basemark.com

  Released under the BSD-2-Clause License
**/

#include <Common/Cpu.h>
#include <SecMain.h>

.global _ModuleEntryPoint
.section .text._ModuleEntryPoint, "ax"
.type _ModuleEntryPoint, %function

_ModuleEntryPoint:
	;
	; Inspired by u-boot arch/arc/lib/start.S
	;

	; Disable/enable I-cache according to configuration
	lr	r5, [ARC_BCR_IC_BUILD]
	breq	r5, 0, 1f ; I$ doesn't exist
	lr	r5, [ARC_AUX_IC_CTRL]
	bset	r5, r5, 0 ; I$ exists, but is not used

	sr	r5, [ARC_AUX_IC_CTRL]

	mov	r5, 1
	sr	r5, [ARC_AUX_IC_IVIC]
	; As per ARC HS databook (see chapter 5.3.3.2)
	; it is required to add 3 NOPs after each write to IC_IVIC.
	nop
	nop
	nop
1:
	; Disable/enable D-cache according to configuration
	lr	r5, [ARC_BCR_DC_BUILD]
	breq	r5, 0, 1f ; D$ doesn't exist
	lr	r5, [ARC_AUX_DC_CTRL]
	bclr	r5, r5, 6 ; Invalidate (discard w/o wback)
	bset	r5, r5, 0 ; Disable (+Inv)
	sr	r5, [ARC_AUX_DC_CTRL]

	mov	r5, 1
	sr	r5, [ARC_AUX_DC_IVDC]
1:
	; ARCv2 only {
	; Disable System-Level Cache (SLC)
	lr	r5, [ARC_BCR_SLC]
	breq	r5, 0, 1f ; SLC doesn't exist
	lr	r5, [ARC_AUX_SLC_CTRL]
	bclr	r5, r5, 6 ; Invalidate (discard w/o wback)
	bclr	r5, r5, 0 ; Enable (+Inv)
	sr	r5, [ARC_AUX_SLC_CTRL]
	; } ARCv2 only
1:
	; ARCv2 only {
	; In case of DSP extension presence in HW some instructions
	; (related to integer multiply, multiply-accumulate, and divide
	; operation) executes on this DSP execution unit. So their
	; execution will depend on dsp configuration register (DSP_CTRL)
	; As we want these instructions to execute the same way regardless
	; of DSP presence we need to set DSP_CTRL properly.
	lr	r5, [ARC_AUX_DSP_BUILD]
	bmsk	r5, r5, 7
	breq    r5, 0, 1f
	mov	r5, 0
	sr	r5, [ARC_AUX_DSP_CTRL]
	; } ARCv2 only
1:
#ifdef __ARC_UNALIGNED__
	 ; Enable handling of unaligned access in the CPU as by default
	 ; this HW feature is disabled while GCC starting from 8.1.0
	 ; unconditionally uses it for ARC HS cores.
	flag	1 << STATUS_AD_BIT
#endif

	; Establish C runtime stack and frame
	mov	%sp, SYS_INIT_SP_ADDR
	mov	%fp, %sp

	; Jump to SEC core, ideally it should never return
	bl	SecMain
        ; Something went wrong, freeze CPU
	sleep
