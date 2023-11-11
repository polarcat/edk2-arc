/** @file
  PEI services implementation.

  See MdeModulePkg/Core/Pei/PeiMain.h for detailed description of each function.

  Copyright (c) 2023 Basemark Oy

  Author: Aliaksei Katovich @ basemark.com

  Released under the BSD-2-Clause License
**/

#include "PeiCoreMain.h"
#include <Library/UtilsLib.h>

//
// TODO: Fixup caclulations should take PEIM's file name into account.
//
UINT32
CalcPeimFixup(
  IN OUT STATUS_INFO *Status
  )
{
  return (UINT32) GetFileSection((VOID *) FixedPcdGet32(PcdBootFvBase),
    EFI_SECTION_FREEFORM_SUBTYPE_GUID, EFI_FV_FILETYPE_PEIM, NULL, Status);
}

VOID
FixupPpi(
  IN OUT EFI_PEI_PPI_DESCRIPTOR *PpiDesc,
  UINT32                        PeimFixup
  )
{
    EFI_DXE_IPL_PPI *Ppi;

    Ppi = (EFI_DXE_IPL_PPI *) (PpiDesc->Ppi + PeimFixup);
    PpiDesc->Ppi = Ppi;
    Ppi->Entry += PeimFixup;
    PpiDesc->Guid = ((VOID *) PpiDesc->Guid) + PeimFixup;
}

#define IS_PIC_PPI(Flags)\
    ((Flags & EFI_PEI_PPI_DESCRIPTOR_PIC) ==\
     EFI_PEI_PPI_DESCRIPTOR_PIC)

#define IS_LAST_PPI(Flags)\
    ((Flags & EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST) ==\
     EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST)

EFI_STATUS
EFIAPI
PeiInstallPpi(
  IN CONST EFI_PEI_SERVICES       **PeiServices,
  IN CONST EFI_PEI_PPI_DESCRIPTOR *PpiList
  )
{
  STATUS_INFO StatusInfo;
  UINT32 PeimFixup;
  PEI_CORE_CONTEXT *PeiCoreCtx;
  PEI_PPI_LIST *PpiListPointer;
  UINTN Idx;
  UINTN LastCount;

  if (PpiList == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  PeiCoreCtx = PS_TO_PEI_CONTEXT_PTR(PeiServices);
  PpiListPointer = &PeiCoreCtx->PpiData.PpiList;
  Idx = PpiListPointer->CurrentCount;
  LastCount = Idx;

  if (Idx >= PeiCoreCtx->PpiData.PpiList.MaxCount) {
    return EFI_NOT_FOUND;
  }

  DBG("> Request PPI desc %p PPI %p\n", PpiList, PpiList->Ppi);

  PeimFixup = 0;
  while (1) {
    if ((PpiList->Flags & EFI_PEI_PPI_DESCRIPTOR_PPI) == 0) {
      PpiListPointer->CurrentCount = LastCount;
      StatusInfo.Status = EFI_INVALID_PARAMETER;
      goto err;
    }

    if (PeimFixup == 0 && IS_PIC_PPI(PpiList->Flags)) {
      PeimFixup = CalcPeimFixup(&StatusInfo);
      if (StatusInfo.Status != EFI_SUCCESS) {
        goto err;
      }
      DBG("| PEIM fixup 0x%x\n", PeimFixup);
    }

    PpiListPointer->PpiPtrs[Idx].Ppi = (EFI_PEI_PPI_DESCRIPTOR *) PpiList;
    PpiListPointer->CurrentCount++;

    FixupPpi(PpiListPointer->PpiPtrs[Idx].Ppi, PeimFixup);

    DBG("[%u/%u] Installed PPI %p GUID %g entry %p\n",
      PpiListPointer->CurrentCount, PpiListPointer->CurrentCount,
      PpiListPointer->PpiPtrs[Idx].Ppi->Ppi,
      PpiListPointer->PpiPtrs[Idx].Ppi->Guid,
      ((EFI_DXE_IPL_PPI *) PpiListPointer->PpiPtrs[Idx].Ppi->Ppi)->Entry);

    if (IS_LAST_PPI(PpiList->Flags)) {
      break;
    }

    if (++Idx >= PeiCoreCtx->PpiData.PpiList.MaxCount) {
      StatusInfo.Status = EFI_NOT_FOUND;
      goto err;
    }

    PpiList++;
  };

  return EFI_SUCCESS;

err:
  //
  // Reject entire provided list.
  //
  PpiListPointer->CurrentCount = LastCount;
  LOG("Failed to install PPI %g\n", PpiList->Guid);
  return StatusInfo.Status;
}

EFI_STATUS
EFIAPI
PeiLocatePpi(
  IN CONST EFI_PEI_SERVICES     **PeiServices,
  IN CONST EFI_GUID             *Guid,
  IN UINTN                      Instance,
  IN OUT EFI_PEI_PPI_DESCRIPTOR **PpiDescriptor,
  IN OUT VOID                   **Ppi
  )
{
  UINTN Idx;
  UINTN TmpInstance;
  EFI_GUID *TmpGuid;
  EFI_PEI_PPI_DESCRIPTOR *TmpPpiDesc;
  PEI_CORE_CONTEXT *PeiCoreCtx;

  if (Guid == NULL || Ppi == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  PeiCoreCtx = PS_TO_PEI_CONTEXT_PTR(PeiServices);
  TmpInstance = 0;

  DBG("> Available %u PPIs\n", PeiCoreCtx->PpiData.PpiList.CurrentCount);

  for (Idx = 0; Idx < PeiCoreCtx->PpiData.PpiList.CurrentCount; Idx++) {
    TmpPpiDesc = PeiCoreCtx->PpiData.PpiList.PpiPtrs[Idx].Ppi;

    DBG("| Check PPI %u GUID %g ? %g\n", Idx, TmpPpiDesc->Guid, Guid);

    if (!TmpPpiDesc) {
      continue;
    } else if (!CompareGuids(Guid, TmpPpiDesc->Guid)) {
      continue;
    } else if (TmpInstance != Instance) {
      TmpInstance++;
      continue;
    } else {
      if (PpiDescriptor) {
        *PpiDescriptor = TmpPpiDesc;
      }

      *Ppi = TmpPpiDesc->Ppi;
      return EFI_SUCCESS;
    }
  }

  return EFI_NOT_FOUND;
}

EFI_STATUS
EFIAPI
PeiNotifyPpi (
  IN CONST EFI_PEI_SERVICES           **PeiServices,
  IN CONST EFI_PEI_NOTIFY_DESCRIPTOR  *NotifyList
  )
{
  return EFI_UNSUPPORTED;
}

EFI_STATUS
EFIAPI
PeiFfsFindNextVolume(
  IN CONST EFI_PEI_SERVICES **PeiServices,
  IN UINTN                  Instance,
  IN OUT EFI_PEI_FV_HANDLE  *VolumeHandle
  )
{
  PEI_CORE_CONTEXT *PeiCoreCtx;

  if (VolumeHandle == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  PeiCoreCtx = PS_TO_PEI_CONTEXT_PTR(PeiServices);
  if (Instance < MAX_CORE_FV) {
    *VolumeHandle = PeiCoreCtx->Fv[Instance].FvHandle;
    return EFI_SUCCESS;
  }

  return EFI_NOT_FOUND;
}

EFI_STATUS
EFIAPI
PeiFfsFindNextFile(
  IN CONST EFI_PEI_SERVICES   **PeiServices,
  IN EFI_FV_FILETYPE          SearchType,
  IN CONST EFI_PEI_FV_HANDLE  FvHandle,
  IN OUT EFI_PEI_FILE_HANDLE  *FileHandle
  )
{
  EFI_FIRMWARE_VOLUME_HEADER *Fv;
  EFI_FFS_FILE_HEADER *File;
  EFI_PHYSICAL_ADDRESS Addr; // Address iterator
  EFI_PHYSICAL_ADDRESS Eov; // End of volume
  EFI_PHYSICAL_ADDRESS Eof; // End of file
  GUID_STR GuidStr;

  if (FvHandle == NULL || FileHandle == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Fv = (EFI_FIRMWARE_VOLUME_HEADER *) FvHandle;
  Eov = ToPhysAddr(Fv) + Fv->FvLength;

  if (*FileHandle == NULL) {
    Addr = AlignAddr(ToPhysAddr(Fv) + Fv->HeaderLength, 8);
  } else {
    // Get next file address
    File = (EFI_FFS_FILE_HEADER *) (UINTN) *FileHandle;
    Addr = AlignAddr(Addr + FFS_FILE_SIZE(File), 8);
  }

  while (Addr < Eov) {
    File = (EFI_FFS_FILE_HEADER *) (UINTN) Addr;
    Eof = Addr + FFS_FILE_SIZE(File);

    GuidToAsciiStr(&File->Name, &GuidStr);
    DBG("| Check file at %p type 0x%x name %a\n", File, File->Type, GuidStr.Data);

    if (Eof > Eov) { // Sanity check
      return EFI_VOLUME_CORRUPTED;
    }

    if (File->Type == SearchType) {
      *FileHandle = File;
      return EFI_SUCCESS;
    }

    Addr = AlignAddr(Eof, 8);
  }

  return EFI_NOT_FOUND;
}

EFI_STATUS
EFIAPI
PeiFfsFindSectionData(
  IN CONST EFI_PEI_SERVICES **PeiServices,
  IN EFI_SECTION_TYPE       SectionType,
  IN EFI_PEI_FILE_HANDLE    FileHandle,
  OUT VOID                  **SectionData
  )
{
  STATUS_INFO StatusInfo;
  EFI_FFS_FILE_HEADER *File;
  EFI_PHYSICAL_ADDRESS Eof; // End of file

  if (FileHandle == NULL || SectionData == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  File = (EFI_FFS_FILE_HEADER *) (UINTN) FileHandle;
  Eof = ToPhysAddr(FileHandle) + FFS_FILE_SIZE(File);

  *SectionData = FindSection(SectionType, ToPhysAddr(File + 1), Eof, &StatusInfo);
  if (StatusInfo.Status != EFI_SUCCESS) {
    LOG("Failed to find section, %a, line %u\n",
      StatusToAsciiStr(StatusInfo.Status), StatusInfo.Line);
  }

  return StatusInfo.Status;
}

EFI_STATUS
EFIAPI
PeiFfsGetFileInfo(
  IN EFI_PEI_FILE_HANDLE  FileHandle,
  OUT EFI_FV_FILE_INFO    *FileInfo
  )
{
  EFI_FFS_FILE_HEADER *File;

  if (FileHandle == NULL || FileInfo == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (IS_FFS_FILE2(File)) {
    return EFI_UNSUPPORTED;
  }

  File = (EFI_FFS_FILE_HEADER *) FileHandle;

  CopyMem(&FileInfo->FileName.Data1, &File->Name, sizeof(FileInfo->FileName));

  FileInfo->FileType = File->Type;
  FileInfo->FileAttributes = File->Attributes;
  FileInfo->Buffer = (VOID *) File + sizeof(EFI_FFS_FILE_HEADER);
  FileInfo->BufferSize = FFS_FILE_SIZE(File) - sizeof(EFI_FFS_FILE_HEADER);

  return EFI_SUCCESS;
}
