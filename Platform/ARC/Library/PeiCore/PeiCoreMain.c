/** @file
  PEI core entry point implementation.

  Copyright (c) 2023 Basemark Oy

  Author: Aliaksei Katovich @ basemark.com

  Released under the BSD-2-Clause License
**/

#include <Library/UtilsLib.h>
#include <Library/BaseLib.h>

/**
  This function is a placeholder for ENTRY_POINT property of corresponding inf
  file. EDK2 build system fails when not provided. The name of the function is
  arbitrary.
**/
VOID
EFIAPI
PeiCoreMain(
  IN CONST  EFI_SEC_PEI_HAND_OFF    *SecCoreData,
  IN CONST  EFI_PEI_PPI_DESCRIPTOR  *PpiList,
  IN VOID                           *Context
  )
{
  CpuDeadLoop();
}

/**
  The entry point of PE/COFF Image for the PEI Core.

  See UEFI PI Spec 1.8, "I-9.2. PEI Foundation Entry Point" for details.

  @param SecCoreData  Pointer to data structure containing information about
                      the PEI core's operating environment.

  @param PpiList      Pointer to one or more PPI descriptors.

  @return             This function should not return.

**/
VOID
EFIAPI
_ModuleEntryPoint(
  IN CONST  EFI_SEC_PEI_HAND_OFF    *SecCoreData,
  IN CONST  EFI_PEI_PPI_DESCRIPTOR  *PpiList
  )
{
  PeiCoreMain(SecCoreData, PpiList, NULL);
}
