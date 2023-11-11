/** @file
  ARC SEC phase module.

  Copyright (c) 2023 Basemark Oy

  Author: Aliaksei Katovich @ basemark.com

  Released under the BSD-2-Clause License
**/

#include <Library/UtilsLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/SerialPortLib.h>
#include <Library/PrintLib.h>

typedef struct {
  UINT8 ZeroVector[16];
  EFI_GUID FileSystemGuid;
  UINT64 FvLength;
  UINT32 Signature;
  EFI_FVB_ATTRIBUTES_2 Attributes;
  UINT16 HeaderLength;
} FV_HEADER;

// The BFV can only be constructed of type EFI_FIRMWARE_FILE_SYSTEM2_GUID
// UEFI PI 1.8: I-9.1.2.3
BOOLEAN
IsValidBootFv(
  IN EFI_FIRMWARE_VOLUME_HEADER *BootFv
  )
{
  return CompareGuid(&BootFv->FileSystemGuid, &gEfiFirmwareFileSystem2Guid);
}

/**
  The entry point of SEC Image.

  UEFI PI 1.8: I-17.1 Security (SEC) phase information.

  @return This function should not return.

**/
VOID
SecMain(VOID)
{
  EFI_PEI_CORE_ENTRY_POINT PeiEp;
  EFI_FIRMWARE_VOLUME_HEADER BootFv;
  UINTN SerialRegisterBase;
  FV_HEADER BootFvHdr;
  STATUS_INFO StatusInfo;
  EFI_SEC_PEI_HAND_OFF SecData;

  CopyMem(&BootFvHdr, (VOID *) FixedPcdGet32(PcdBootFvBase), sizeof(BootFvHdr));
  CopyMem(&BootFv, (VOID *) &BootFvHdr, BootFvHdr.HeaderLength);

  SerialRegisterBase = SerialPortInitialize();
  LOG("Enter SEC, boot FV addr %p size %u\n", &BootFv, BootFv.HeaderLength);

  if (BootFv.Signature != EFI_FVH_SIGNATURE) {
    LOG("Bad FV signature at 0x%x\n", BootFv.Signature);
    goto halt;
  }

  if (!IsValidBootFv(&BootFv)) {
    LOG("Not a Boot FV at %p\n", &BootFv);
    goto halt;
  }

  DBG("Boot FV at %p\n", &BootFv);
  ZeroMem(&SecData, sizeof(SecData));

  SecData.DataSize = sizeof(EFI_SEC_PEI_HAND_OFF);
  SecData.BootFirmwareVolumeBase = FixedPcdGet32(PcdBootFvBase);
  SecData.BootFirmwareVolumeSize = (UINTN) BootFv.FvLength;

  PeiEp = GetTeEntryPoint(SecData.BootFirmwareVolumeBase,
    EFI_FV_FILETYPE_PEI_CORE, NULL, &StatusInfo);
  if (PeiEp == NULL) {
    LOG("Failed to find PEI core entry point | Status '%a' %u\n",
      StatusToAsciiStr(StatusInfo.Status), StatusInfo.Line);
    goto halt;
  }

  SecData.TemporaryRamBase = (VOID *) FixedPcdGet32(PcdPeiTemporaryRamBase);
  SecData.TemporaryRamSize = (UINTN) FixedPcdGet32(PcdPeiTemporaryRamSize);

  SecData.PeiTemporaryRamBase = SecData.TemporaryRamBase;
  SecData.PeiTemporaryRamSize = SecData.TemporaryRamSize >> 1;

  SecData.StackBase = SecData.TemporaryRamBase;
  SecData.StackBase += SecData.TemporaryRamSize >> 1;
  SecData.StackSize = SecData.TemporaryRamSize >> 1;

  LOG("Call PEI core at %p\n", PeiEp);

  PeiEp(&SecData, NULL);

  LOG("Fatal error: returned from PEI entry point\n");

halt:
  LOG("-= Boot failed =-\n");
}
