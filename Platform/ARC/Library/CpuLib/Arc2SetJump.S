/** @file
  ARC Set jump implementation.

  Copyright (c) 2023 Basemark Oy

  Author: Aliaksei Katovich @ basemark.com

  Released under the BSD-2-Clause License
**/

.global SetJump
.section .text.SetJump, "ax"
.type SetJump, %function

SetJump:
	; return
	j_s     [blink]
	nop_s

.global InternalLongJump
.section .text.InternalLongJump, "ax"
.type InternalLongJump, %function

InternalLongJump:
	; return
	j_s     [blink]
	nop_s
