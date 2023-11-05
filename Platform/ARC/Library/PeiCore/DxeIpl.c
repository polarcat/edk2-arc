/** @file
  Last PEIM executed in PEI phase to load DXE Core from a Firmware Volume.

  UEFI PI 1.8: I-13. PEI to DXE handoff.

  Copyright (c) 2023 Basemark Oy

  Author: Aliaksei Katovich @ basemark.com

  Released under the BSD-2-Clause License
**/

#include <Library/UtilsLib.h>
#include <Ppi/DxeIpl.h>
#include <Core/Pei/PeiMain.h>

/**
  DXE IPL PEIM main function.

  UEFI PI 1.8: I-9.8.2.7 Invoking the PEIM’s Entry Point.

  @param  FileHandle  Handle of the file being invoked.
  @param  PeiServices Describes the list of possible PEI Services.

  @return EFI_STATUS  EFI_SUCESS on success,

**/
EFI_STATUS
DxeIplMain(
  IN CONST EFI_DXE_IPL_PPI *This,
  IN EFI_PEI_SERVICES      **PeiServices,
  IN EFI_PEI_HOB_POINTERS  HobList
  )
{
  EFI_STATUS Status;

  LOG("Enter DXE IPL\n");

  return EFI_SUCCESS;
}

EFI_DXE_IPL_PPI mDxeIplPpi = {
  DxeIplMain
};

CONST EFI_PEI_PPI_DESCRIPTOR mDxeIplPpiList[] = {
  {
    EFI_PEI_PPI_DESCRIPTOR_PPI_PIC | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST,
    &gEfiDxeIplPpiGuid,
    &mDxeIplPpi,
  },
};

/**
  DXE IPL PEIM init function.

  @param  FileHandle  Handle of the file being invoked.
  @param  PeiServices Describes the list of possible PEI Services.

  @return EFI_STATUS  EFI_SUCESS on success,

**/
EFI_STATUS
DxeIplInit(
  IN       EFI_PEI_FILE_HANDLE  FileHandle,
  IN CONST EFI_PEI_SERVICES     **PeiServices
  )
{
  CONST EFI_PEI_PPI_DESCRIPTOR *PpiList = mDxeIplPpiList;

  LOG("Init DXE IPL, mDxeIplPpiList %p, mDxeIplPpi %p DxeIplMain %p Ppi %p Guid %g\n",
    mDxeIplPpiList, &mDxeIplPpi, DxeIplMain, PpiList->Ppi, PpiList->Guid);

  return (*PeiServices)->InstallPpi(PeiServices, mDxeIplPpiList);
}
