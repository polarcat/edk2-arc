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

#define DXE_FV_INSTANCE 1

EFI_GUID mEfiDxeIplPpiGuid = EFI_DXE_IPL_PPI_GUID;

// TODO: move these to own PEIM

CONST EFI_PEI_SERVICES **mPs;

EFI_STATUS
PeiLoadPe32(
  VOID                      *Pe32Data,
  OUT EFI_PHYSICAL_ADDRESS  *ImageAddress,
  OUT UINT64                *ImageSize,
  OUT EFI_PHYSICAL_ADDRESS  *EntryPoint
  )
{
  EFI_STATUS Status;
  EFI_IMAGE_DOS_HEADER *DosHdr = (EFI_IMAGE_DOS_HEADER *) Pe32Data;
  EFI_IMAGE_NT_HEADERS32 *Pe32Hdr;

  if (DosHdr->e_magic != EFI_IMAGE_DOS_SIGNATURE) {
    LOG("Bad image signature %x, line %u\n", DosHdr->e_magic, __LINE__);
    return EFI_LOAD_ERROR;
  }

  Pe32Hdr = (EFI_IMAGE_NT_HEADERS32 *) (Pe32Data + DosHdr->e_lfanew);
  if (Pe32Hdr->Signature != EFI_IMAGE_NT_SIGNATURE) {
    LOG("Bad image signature %x, line %u\n", Pe32Hdr->Signature, __LINE__);
    return EFI_LOAD_ERROR;
  }

  if (Pe32Hdr->OptionalHeader.Magic != EFI_IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
    LOG("Bad image signature %x, line %u\n", Pe32Hdr->OptionalHeader.Magic,
      __LINE__);
    return EFI_LOAD_ERROR;
  }

#if 0
  if (Pe32Hdr->FileHeader.Machine != EFI_IMAGE_MACHINE_ARC2) {
    LOG("Unknown machine %x, line %u\n", Pe32Hdr->FileHeader.Machine,
      __LINE__);
    return EFI_LOAD_ERROR;
  }
#endif

  //
  // XIP image starts at the same address as corresponding file section.
  //
  *ImageAddress = ToPhysAddr(Pe32Data);
  *ImageSize = Pe32Hdr->OptionalHeader.SizeOfImage;
  *EntryPoint = ToPhysAddr(
      Pe32Hdr->OptionalHeader.AddressOfEntryPoint + Pe32Data
    );

  LOG("Opt. header:   %p\n", &Pe32Hdr->OptionalHeader);
  LOG("Machine:       %x\n", Pe32Hdr->FileHeader.Machine);
  LOG("Magic:         %x\n", Pe32Hdr->OptionalHeader.Magic);
  LOG("Code size:     %u\n", Pe32Hdr->OptionalHeader.SizeOfCode);
  LOG("Data size (I): %u\n", Pe32Hdr->OptionalHeader.SizeOfInitializedData);
  LOG("Data size (U): %u\n", Pe32Hdr->OptionalHeader.SizeOfUninitializedData);
  LOG("Entry:         %x\n", Pe32Hdr->OptionalHeader.AddressOfEntryPoint);
  LOG("Code:          %x\n", Pe32Hdr->OptionalHeader.BaseOfCode);
  LOG("Data:          %x\n", Pe32Hdr->OptionalHeader.BaseOfData);
  LOG("Image base:    %x\n", Pe32Hdr->OptionalHeader.ImageBase);
  LOG("Image size:    %u\n", Pe32Hdr->OptionalHeader.SizeOfImage);
  LOG("Alignment:     %u\n", Pe32Hdr->OptionalHeader.SectionAlignment);

  return EFI_SUCCESS;
}

EFI_STATUS
PeiLoadFile(
  IN CONST EFI_PEI_LOAD_FILE_PPI  *This,
  IN EFI_PEI_FILE_HANDLE          FileHandle,
  OUT EFI_PHYSICAL_ADDRESS        *ImageAddress,
  OUT UINT64                      *ImageSize,
  OUT EFI_PHYSICAL_ADDRESS        *EntryPoint,
  OUT UINT32                      *AuthenticationState
  )
{
  EFI_STATUS Status;
  VOID *Pe32Data;

  if (ImageAddress == NULL || ImageSize == NULL ||
    EntryPoint == NULL || AuthenticationState == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = (*mPs)->FfsFindSectionData(mPs, EFI_SECTION_PE32, FileHandle,
    &Pe32Data);
  if (Status != EFI_SUCCESS) {
    return Status;
  }
  LOG("Found DXE PE32 section at %p\n", Pe32Data);

  Pe32Data += sizeof(EFI_COMMON_SECTION_HEADER);
  return PeiLoadPe32(Pe32Data, ImageAddress, ImageSize, EntryPoint);
}

VOID
LoadAndRunDxeCore(
  IN EFI_PEI_FILE_HANDLE File
  )
{
  SWITCH_STACK_ENTRY_POINT DxeCoreMain;
  EFI_STATUS Status;
  EFI_PHYSICAL_ADDRESS Addr;
  UINT64 Size;
  EFI_PHYSICAL_ADDRESS Entry;
  UINT32 Auth;

  Status = PeiLoadFile(NULL, File, &Addr, &Size, &Entry, &Auth);
  if (Status != EFI_SUCCESS) {
    LOG("Failed to load DXE core, %s\n", StatusToAsciiStr(Status));
    CpuDeadLoop();
  }

  LOG("DXE core entry point %x\n", Entry);
#if 0
  BuildModuleHob(&FileInfo.FileName, Addr, ALIGN_VALUE(Size, EFI_PAGE_SIZE),
    Entry);
#endif

  DxeCoreMain = (SWITCH_STACK_ENTRY_POINT) (VOID *) Entry;
//  DxeCoreMain(NULL, NULL);
}

/**
  DXE IPL PEIM main function.

  UEFI PI 1.8: I-9.8.2.7 Invoking the PEIM’s Entry Point.

  @param  FileHandle  Handle of the file being invoked.
  @param  PeiServices Describes the list of possible PEI Services.

  @return EFI_STATUS  EFI_SUCESS on success,

**/
EFI_STATUS
DxeIplMain(
  IN CONST EFI_DXE_IPL_PPI  *This,
  IN EFI_PEI_SERVICES       **PeiServices,
  IN EFI_PEI_HOB_POINTERS   HobList
  )
{
  EFI_STATUS Status;
  EFI_PEI_FV_HANDLE Fv;
  EFI_PEI_FILE_HANDLE File;
  EFI_FV_FILE_INFO FileInfo;
  CONST EFI_PEI_SERVICES **Ps = (CONST EFI_PEI_SERVICES **) PeiServices;

  LOG("Enter DXE IPL\n");

  Status = (*Ps)->FfsFindNextVolume(Ps, DXE_FV_INSTANCE, &Fv);
  if (Status != EFI_SUCCESS) {
    return Status;
  }

  File = NULL;
  Status = (*Ps)->FfsFindNextFile(Ps, EFI_FV_FILETYPE_DXE_CORE, Fv, &File);
  if (Status != EFI_SUCCESS) {
    return Status;
  }

  LOG("Found DXE core file at %p\n", File);

  Status = (*Ps)->FfsGetFileInfo(File, &FileInfo);
  if (Status == EFI_SUCCESS) {
    // Should never return.
    LOG("Load DXE file %g\n", &FileInfo.FileName);
    LoadAndRunDxeCore(File);
  }

  return EFI_LOAD_ERROR;
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
  LOG("Init DXE IPL, mDxeIplPpiList %p, mDxeIplPpi %p DxeIplMain %p\n",
    mDxeIplPpiList, &mDxeIplPpi, DxeIplMain);

  mPs = PeiServices;
  return (*PeiServices)->InstallPpi(PeiServices, mDxeIplPpiList);
}
