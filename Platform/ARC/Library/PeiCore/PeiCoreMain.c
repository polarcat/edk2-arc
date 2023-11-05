/** @file
  PEI core entry point implementation.

  Copyright (c) 2023 Basemark Oy

  Author: Aliaksei Katovich @ basemark.com

  Released under the BSD-2-Clause License
**/

#include <Library/UtilsLib.h>
#include <Library/BaseLib.h>
#include <Library/PeiServicesTablePointerLib.h>
#include <Core/Pei/PeiMain.h>

#ifndef MAX_PEI_PPIS
#define MAX_PEI_PPIS 256
#endif

PEI_PPI_LIST_POINTERS mPpiListPool[MAX_PEI_PPIS];

PEI_CORE_INSTANCE gPeiCoreInstance = {
  .Signature = PEI_CORE_HANDLE_SIGNATURE,
  .ServiceTableShadow = {
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
  },
  .PpiData.PpiList = {
    .CurrentCount = 0,
    .MaxCount = ARRAY_SIZE(mPpiListPool),
    .LastDispatchedCount = 0,
    .PpiPtrs = mPpiListPool,
  },
};

UINT32
CalcPeiFixup(
  IN VOID *FvBase
  )
{
  return (UINT32) GetFileSection(FvBase, EFI_SECTION_FREEFORM_SUBTYPE_GUID,
    EFI_FV_FILETYPE_PEI_CORE, NULL, NULL);
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
      StatusToAsciiStr(StatusInfo.Status), StatusInfo.SourceCodeLine);
    return;
  }

  //
  // TODO: Handle all GUIDs from apriori file
  //
  FileName = &AprioriFile->FileNamesWithinVolume[0];
  LOG("PEIM %g\n", FileName);

  PeimInit = GET_ENTRY_POINT(FileName);
  if (PeimInit != NULL) {
    LOG("Init PEIM at %p, %a | %u\n", PeimInit,
      StatusToAsciiStr(StatusInfo.Status), StatusInfo.SourceCodeLine);
    Status = PeimInit(NULL, (CONST EFI_PEI_SERVICES **) &gPeiCoreInstance.Ps);
    LOG("| Status %a\n", StatusToAsciiStr(Status));
  }
}

VOID
DispatchPeims(VOID)
{
  EFI_STATUS Status;
  UINTN Idx;
  EFI_PEI_PPI_DESCRIPTOR *TmpPpiDesc;
  EFI_DXE_IPL_ENTRY Entry;
  CONST EFI_DXE_IPL_PPI *Ppi;
  EFI_PEI_HOB_POINTERS HobPtrs;

  LOG("Available %u PPIs\n", gPeiCoreInstance.PpiData.PpiList.CurrentCount);

  for (Idx = 0; Idx < gPeiCoreInstance.PpiData.PpiList.CurrentCount; Idx++) {
    TmpPpiDesc = gPeiCoreInstance.PpiData.PpiList.PpiPtrs[Idx].Ppi;

    Ppi = TmpPpiDesc->Ppi;
    LOG("Run PPI %u GUID %g entry %p\n", Idx, TmpPpiDesc->Guid, Ppi->Entry);
    Status = Ppi->Entry(TmpPpiDesc->Ppi, &gPeiCoreInstance.Ps, HobPtrs);
    LOG("| Status %a\n", StatusToAsciiStr(Status));
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
  UINT32 PeiFixup;
  EFI_STATUS Status;
  EFI_DXE_IPL_PPI DxeIpl;

  PeiFixup = CalcPeiFixup(SecCoreData->BootFirmwareVolumeBase);

  LOG("Enter PEI CORE, instance %p, fixup 0x%x\n", &gPeiCoreInstance, PeiFixup);

  gPeiCoreInstance.Ps = &gPeiCoreInstance.ServiceTableShadow;

  //
  // Fix up addresses of PEI services
  //
  gPeiCoreInstance.Ps->InstallPpi += PeiFixup;
  gPeiCoreInstance.Ps->LocatePpi += PeiFixup;
  gPeiCoreInstance.Ps->NotifyPpi += PeiFixup;

  SetPeiServicesTablePointer((CONST EFI_PEI_SERVICES **) &gPeiCoreInstance.Ps);
  InitPeims(SecCoreData->BootFirmwareVolumeBase);
  DispatchPeims();

#if 0
  Status = PeiLocatePpi((CONST EFI_PEI_SERVICES **) &gPeiCoreInstance.Ps,
    &gEfiDxeIplPpiGuid, 0, NULL, (VOID **) &DxeIpl);
  if (Status != EFI_SUCCESS) {
    LOG("Failed to locate DXE IPL service, %a\n", StatusToAsciiStr(Status));
  } else {
    LOG("DXE IPL entry %p\n", DxeIpl);
  }
#endif
  CpuDeadLoop();
}
