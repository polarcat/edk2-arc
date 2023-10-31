#pragma once

/* Instruction cache related auxiliary registers */
#define ARC_AUX_IC_IVIC 0x10
#define ARC_AUX_IC_CTRL 0x11
#define ARC_BCR_IC_BUILD 0x77

/* Data cache related auxiliary registers */
#define ARC_AUX_DC_IVDC 0x47
#define ARC_AUX_DC_CTRL 0x48
#define ARC_BCR_DC_BUILD 0x72
#define ARC_BCR_SLC 0xce

/* DSP-extensions related auxiliary registers */
#define ARC_AUX_DSP_BUILD 0x7a
#define ARC_AUX_DSP_CTRL 0x59f
#define ARC_AUX_SLC_CTRL 0x903

/* STATUS32 Bits Positions */
#define STATUS_AD_BIT 19 // Enable unaligned access

/* Default stack address */
#define SYS_INIT_SP_ADDR 0x80000f30
