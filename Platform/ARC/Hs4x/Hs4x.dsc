[Defines]
  PLATFORM_NAME = hs4x
  FLASH_DEFINITION = Platform/ARC/Hs4x/Hs4x.fdf
  # The rest of defines will come from include file below.

#
# Include file [Defines] section will be complemented with defines above.
#
!include Platform/ARC/Arc.dsc.inc

[BuildOptions.ARC2]
  GCC:*_*_*_CC_FLAGS = -mcpu=hs4x -DMDE_CPU_ARC2
  GCC:*_*_*_ASM_FLAGS = -mcpu=hs4x -DMDE_CPU_ARC2
  GCC:*_*_*_PP_FLAGS = -D__ASSEMBLY__ -mcpu=hs4x -DMDE_CPU_ARC2
