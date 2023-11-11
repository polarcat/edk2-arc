/** @file
  PEI core entry point implementation.

  Copyright (c) 2023 Basemark Oy

  Author: Aliaksei Katovich @ basemark.com

  Released under the BSD-2-Clause License
**/

#include "PeiCoreMain.h"
#include <Library/UtilsLib.h>
#include <Library/BaseLib.h>
#include <Library/PeiServicesTablePointerLib.h>

#ifndef MAX_PEI_PPIS
#define MAX_PEI_PPIS 256
#endif

#define MAX_CORE_FV 2

PEI_PPI_LIST_POINTERS mPpiListPool[MAX_PEI_PPIS];

PEI_CORE_CONTEXT mPeiCoreCtx = {
  .Ps = {
    .Hdr = {
      .Signature = PEI_SERVICES_SIGNATURE,
      .Revision = PEI_SERVICES_REVISION,
      .HeaderSize = sizeof(EFI_PEI_SERVICES),
      .CRC32 = 0,
      .Reserved = 0,
    },
    .InstallPpi = PeiInstallPpi,
    .LocatePpi = PeiLocatePpi,
    .NotifyPpi = PeiNotifyPpi,
    .FfsFindNextVolume = PeiFfsFindNextVolume,
    .FfsFindNextFile = PeiFfsFindNextFile,
    .FfsFindSectionData = PeiFfsFindSectionData,
    .FfsGetFileInfo = PeiFfsGetFileInfo,
  },
  .PpiData.PpiList = {
    .CurrentCount = 0,
    .MaxCount = ARRAY_SIZE(mPpiListPool),
    .LastDispatchedCount = 0,
    .PpiPtrs = mPpiListPool,
  },
};

STATIC UINT32 mPeiFixup;

inline
EFI_FIRMWARE_VOLUME_HEADER *
VoidToFvHdr(
  VOID *Ptr
  )
{
  return (EFI_FIRMWARE_VOLUME_HEADER *) Ptr;
}

inline
EFI_FIRMWARE_VOLUME_HEADER *
IntToFvHdr(
  UINTN Int
  )
{
  return (EFI_FIRMWARE_VOLUME_HEADER *) Int;
}

UINT32
CalcPeiFixup(
  IN VOID *FvBase
  )
{
  return (UINT32) GetFileSection(FvBase, EFI_SECTION_FREEFORM_SUBTYPE_GUID,
    EFI_FV_FILETYPE_PEI_CORE, NULL, NULL);
}

PEI_CORE_CONTEXT *
GetCorePeiInstance(
  IN CONST EFI_PEI_SERVICES **PeiServices
  )
{
  //VOID *Ptr = PEI_CORE_INSTANCE_FROM_PS_THIS(PeiServices);
  //return (PEI_CORE_INSTANCE *) (Ptr + mPeiFixup);
  return &mPeiCoreCtx;
}

#define GET_ENTRY_POINT(FileName)\
  GetTeEntryPoint(FvBase, EFI_FV_FILETYPE_PEIM, FileName, &StatusInfo);

VOID
InitPeims(
  IN VOID *FvBase
  )
{
  EFI_STATUS Status;
  STATUS_INFO StatusInfo;
  EFI_PEIM_ENTRY_POINT2 PeimInit;
  PEI_APRIORI_FILE_CONTENTS *AprioriFile;
  EFI_GUID *FileName;

  AprioriFile = GetAprioriFile(FvBase, &StatusInfo);
  if (AprioriFile == NULL) {
    LOG("Failed to find apriori file, %a | %u\n",
      StatusToAsciiStr(StatusInfo.Status), StatusInfo.Line);
    return;
  }

  //
  // TODO: Handle all GUIDs from apriori file
  //
  FileName = &AprioriFile->FileNamesWithinVolume[0];
  DBG("> Handle PEIM file %g\n", FileName);

  PeimInit = GET_ENTRY_POINT(FileName);
  if (PeimInit != NULL) {
    DBG("| Call PEIM's init at %p, %a | %u\n", PeimInit,
      StatusToAsciiStr(StatusInfo.Status), StatusInfo.Line);
    Status = PeimInit(NULL, (CONST EFI_PEI_SERVICES **) &mPeiCoreCtx.PsPtr);
    DBG("| Status %a\n", StatusToAsciiStr(Status));
  }
}

VOID
DispatchPeims(VOID)
{
  EFI_STATUS Status;
  UINTN Idx;
  EFI_PEI_PPI_DESCRIPTOR *TmpPpiDesc;
  CONST EFI_DXE_IPL_PPI *Ppi;
  EFI_PEI_HOB_POINTERS HobList;

  DBG("Available %u PPIs\n", mPeiCoreCtx.PpiData.PpiList.CurrentCount);

  // TODO: make sure DXE IPL called last

  for (Idx = 0; Idx < mPeiCoreCtx.PpiData.PpiList.CurrentCount; Idx++) {
    TmpPpiDesc = mPeiCoreCtx.PpiData.PpiList.PpiPtrs[Idx].Ppi;

    Ppi = TmpPpiDesc->Ppi;
    DBG("[%u] Run PPI GUID %g entry %p\n", Idx, TmpPpiDesc->Guid, Ppi->Entry);
    Status = Ppi->Entry(TmpPpiDesc->Ppi, &mPeiCoreCtx.PsPtr, HobList);
    DBG("| Status %a\n", StatusToAsciiStr(Status));
  }
}

/**
  This routine is invoked by main entry of PeiMain module during transition
  from SEC to PEI. After switching stack in the PEI core, it will restart
  with the old core data.

  UEFI PI 1.8: I-9.2. PEI Foundation Entry Point.

  @param SecCoreData  Pointer to data structure containing information about
                      the PEI core's operating environment.

  @param PpiList      Pointer to one or more PPI descriptors.

  @param Data         Pointer to the old core data that is used to initialize the
                      core's data areas. If NULL, it is first PeiCore entering.

  @return             This function should not return.

**/
VOID
PeiCoreMain(
  IN CONST  EFI_SEC_PEI_HAND_OFF    *SecCoreData,
  IN CONST  EFI_PEI_PPI_DESCRIPTOR  *PpiList,
  IN VOID                           *Data OPTIONAL
  )
{
  VOID *TmpPtr;
  EFI_STATUS Status;
  EFI_DXE_IPL_PPI DxeIpl;

  mPeiFixup = CalcPeiFixup(SecCoreData->BootFirmwareVolumeBase);

  LOG("Enter PEI CORE, instance %p, fixup 0x%x\n", &mPeiCoreCtx, mPeiFixup);

  mPeiCoreCtx.PsPtr = &mPeiCoreCtx.Ps;

  //
  // Fix up addresses of PEI services
  //
  mPeiCoreCtx.PsPtr->InstallPpi += mPeiFixup;
  mPeiCoreCtx.PsPtr->LocatePpi += mPeiFixup;
  mPeiCoreCtx.PsPtr->NotifyPpi += mPeiFixup;
  mPeiCoreCtx.PsPtr->FfsFindNextVolume += mPeiFixup;
  mPeiCoreCtx.PsPtr->FfsFindNextFile += mPeiFixup;
  mPeiCoreCtx.PsPtr->FfsFindSectionData += mPeiFixup;
  mPeiCoreCtx.PsPtr->FfsGetFileInfo += mPeiFixup;

  //
  // Fill in BOOT and DXE FV info
  //
  mPeiCoreCtx.Fv[0].FvHeader = VoidToFvHdr(SecCoreData->BootFirmwareVolumeBase);
  mPeiCoreCtx.Fv[0].FvHandle = (VOID *) mPeiCoreCtx.Fv[0].FvHeader;

  mPeiCoreCtx.Fv[1].FvHeader = IntToFvHdr(FixedPcdGet32(PcdDxeFvBase));
  mPeiCoreCtx.Fv[1].FvHandle = (VOID *) mPeiCoreCtx.Fv[1].FvHeader;

  SetPeiServicesTablePointer((CONST EFI_PEI_SERVICES **) &mPeiCoreCtx.PsPtr);
  InitPeims(SecCoreData->BootFirmwareVolumeBase);
  DispatchPeims();

  CpuDeadLoop();
}
