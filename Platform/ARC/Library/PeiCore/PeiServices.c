/** @file
  PEI services implementation.

  See MdeModulePkg/Core/Pei/PeiMain.h for detailed description of each function.

  Copyright (c) 2023 Basemark Oy

  Author: Aliaksei Katovich @ basemark.com

  Released under the BSD-2-Clause License
**/

#include <Library/UtilsLib.h>
#include <Core/Pei/PeiMain.h>

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
  UINT32 PeimFixup
  )
{
    EFI_DXE_IPL_PPI *Ppi;

    Ppi = (EFI_DXE_IPL_PPI *) (PpiDesc->Ppi + PeimFixup);
    PpiDesc->Ppi = Ppi;
    Ppi->Entry += PeimFixup;
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
  IN CONST EFI_PEI_SERVICES        **PeiServices,
  IN CONST EFI_PEI_PPI_DESCRIPTOR  *PpiList
  )
{
  STATUS_INFO StatusInfo;
  UINT32 PeimFixup;
  PEI_CORE_INSTANCE *PeiCore;
  PEI_PPI_LIST *PpiListPointer;
  UINTN Idx;
  UINTN LastCount;

  if (PpiList == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  PeiCore = PEI_CORE_INSTANCE_FROM_PS_THIS(PeiServices);
  PpiListPointer = &PeiCore->PpiData.PpiList;
  Idx = PpiListPointer->CurrentCount;
  LastCount = Idx;

  if (Idx >= PeiCore->PpiData.PpiList.MaxCount) {
    return EFI_OUT_OF_RESOURCES;
  }

  PeimFixup = 0;
  LOG("> Request PPI desc %p PPI %p\n", PpiList, PpiList->Ppi);
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
      LOG("| PEIM fixup 0x%x\n", PeimFixup);
    }

    PpiListPointer->PpiPtrs[Idx].Ppi = (EFI_PEI_PPI_DESCRIPTOR *) PpiList;
    PpiListPointer->CurrentCount++;

    FixupPpi(PpiListPointer->PpiPtrs[Idx].Ppi, PeimFixup);

    LOG("[%u/%u] Installed PPI %p entry %p\n", PpiListPointer->CurrentCount,
      PpiListPointer->CurrentCount, PpiListPointer->PpiPtrs[Idx].Ppi->Ppi,
      ((EFI_DXE_IPL_PPI *) PpiListPointer->PpiPtrs[Idx].Ppi->Ppi)->Entry);

    if (IS_LAST_PPI(PpiList->Flags)) {
      break;
    }

    if (++Idx >= PeiCore->PpiData.PpiList.MaxCount) {
      StatusInfo.Status = EFI_OUT_OF_RESOURCES;
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
  IN CONST EFI_PEI_SERVICES      **PeiServices,
  IN CONST EFI_GUID              *Guid,
  IN UINTN                       Instance,
  IN OUT EFI_PEI_PPI_DESCRIPTOR  **PpiDescriptor,
  IN OUT VOID                    **Ppi
  )
{
  UINTN Idx;
  UINTN TmpInstance;
  EFI_GUID *TmpGuid;
  EFI_PEI_PPI_DESCRIPTOR *TmpPpiDesc;
  PEI_CORE_INSTANCE *PeiCore;

  if (PeiServices == NULL || Guid == NULL || Ppi == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  PeiCore = PEI_CORE_INSTANCE_FROM_PS_THIS(PeiServices);
  TmpInstance = 0;

  LOG("Available %u PPIs\n", PeiCore->PpiData.PpiList.CurrentCount);

  for (Idx = 0; Idx < PeiCore->PpiData.PpiList.CurrentCount; Idx++) {
    TmpPpiDesc = PeiCore->PpiData.PpiList.PpiPtrs[Idx].Ppi;

    LOG("Check PPI %u GUID %g ? %g\n", Idx, TmpPpiDesc->Guid, Guid);

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
