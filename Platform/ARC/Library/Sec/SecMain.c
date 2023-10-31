/** @file
  ARC SEC phase module.

  Copyright (c) 2023 Basemark Oy

  Author: Aliaksei Katovich @ basemark.com

  Released under the BSD-2-Clause License
**/

#include <SecMain.h>
#include <Library/UtilsLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PeCoffExtraActionLib.h>
#include <Library/PeCoffGetEntryPointLib.h>
#include <Library/PeCoffLib.h>
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

typedef struct {
  EFI_SEC_PEI_HAND_OFF SecData;
  VOID *PeiServiceTable;
  EFI_STATUS Status;
  UINT32 Line; // Track source code line number
} SEC_CONTEXT;

#define SET_STATUS(Ctx, Val) {\
  Ctx->Line = __LINE__;\
  Ctx->Status = Val;\
}

// The BFV can only be constructed of type EFI_FIRMWARE_FILE_SYSTEM2_GUID
// UEFI PI 1.8, I-9.1.2.3
BOOLEAN
IsValidBootFv(
  IN EFI_FIRMWARE_VOLUME_HEADER *BootFv
  )
{
  LOG("Boot FV addr %p\n", BootFv);
  return CompareGuid(&BootFv->FileSystemGuid, &gEfiFirmwareFileSystem2Guid);
}

// ----------------------------------------------------------------------------
// Firmware volume helpers
// ----------------------------------------------------------------------------

typedef struct {
  SEC_CONTEXT *Ctx;
  EFI_FV_FILETYPE FileType;
  EFI_SECTION_TYPE SectionType;
} FILE_SECTION_REQ;

typedef struct {
  SEC_CONTEXT *Ctx;
  EFI_SECTION_TYPE SectionType;
  EFI_PHYSICAL_ADDRESS SectionsAddr;
  EFI_PHYSICAL_ADDRESS SectionsEnd;
} SECTION_REQ;

VOID *
ToVoidPtr(
  IN EFI_PHYSICAL_ADDRESS Addr
  )
{
  return (VOID *) (UINTN) Addr;
}

EFI_COMMON_SECTION_HEADER *
ToSectHdr(
  IN EFI_PHYSICAL_ADDRESS Addr
)
{
  return (EFI_COMMON_SECTION_HEADER *) (UINTN) Addr;
}

EFI_FFS_FILE_HEADER *
ToFileHdr(
  IN EFI_PHYSICAL_ADDRESS Addr
  )
{
  return (EFI_FFS_FILE_HEADER *) (UINTN) Addr;
}

EFI_FIRMWARE_VOLUME_HEADER *
ToFvHdr(
  IN VOID *Ptr
  )
{
  return (EFI_FIRMWARE_VOLUME_HEADER *) Ptr;
}

EFI_PHYSICAL_ADDRESS
ToPhysAddr(
  IN VOID *Ptr
  )
{
  return (EFI_PHYSICAL_ADDRESS) (UINTN) Ptr;
}

VOID *
GetSection(
  IN OUT SECTION_REQ *Req
  )
{
  EFI_PHYSICAL_ADDRESS Addr; // Address iterator
  EFI_COMMON_SECTION_HEADER *Section;
  UINT32 Size;

  Addr = (Req->SectionsAddr + 3) & ~3ULL; // 4-byte aligned

  while (Addr < Req->SectionsEnd) {
    LOG("Section at %p | line %d\n", ToVoidPtr(Addr), __LINE__);

    Section = ToSectHdr(Addr);
    Size = SECTION_SIZE(Section);
    LOG("Section size 0x%x type 0x%x\n", Size, Section->Type);
    if (Size < sizeof(*Section)) {
      SET_STATUS(Req->Ctx, EFI_VOLUME_CORRUPTED);
      return NULL;
    }

    if (Addr + Size > Req->SectionsEnd) {
      SET_STATUS(Req->Ctx, EFI_VOLUME_CORRUPTED);
      return NULL;
    }

    if (Section->Type == Req->SectionType) {
        SET_STATUS(Req->Ctx, EFI_SUCCESS);
        return Section;
    }

    Addr += Size;
  }

  SET_STATUS(Req->Ctx, EFI_NOT_FOUND);
  return NULL;
}

VOID *
GetFileSection(
  IN OUT FILE_SECTION_REQ *Req
  )
{
  EFI_PHYSICAL_ADDRESS Addr; // Address iterator
  EFI_PHYSICAL_ADDRESS Eov; // End of volume
  EFI_PHYSICAL_ADDRESS Eof; // End of file
  EFI_FFS_FILE_HEADER *File;
  UINT32 Size;
  EFI_FIRMWARE_VOLUME_HEADER *Fv;
  GUID_STR GuidStr;

  LOG("Enter GetFileSection\n");
  Fv = ToFvHdr(Req->Ctx->SecData.BootFirmwareVolumeBase);
  Eov = ToPhysAddr(Fv) + Fv->FvLength;
  Addr = (ToPhysAddr(Fv) + Fv->HeaderLength + 7) & ~7ULL; // 8-byte aligned

  while (Addr < Eov) {
    File = ToFileHdr(Addr);
    Size = FFS_FILE_SIZE(File);
    Eof = Addr + Size;

    GuidToAsciiStr(&File->Name, &GuidStr);

    LOG("File at %p name %a\n", ToVoidPtr(Addr), GuidStr.Data);

    if (Eof > Eov) { // Sanity check
      SET_STATUS(Req->Ctx, EFI_VOLUME_CORRUPTED);
      return NULL;
    }

    if (File->Type == Req->FileType) {
      SECTION_REQ SectReq = {
        .Ctx = Req->Ctx,
        .SectionType = Req->SectionType,
        .SectionsAddr = ToPhysAddr(File + 1),
        .SectionsEnd = Eof,
      };
      return GetSection(&SectReq);
    }

    Addr = (Eof + 7) & ~7ULL;
  }

  return NULL;
}

VOID *
GetPeiCoreImage(
  IN OUT SEC_CONTEXT *Ctx
  )
{
  VOID *PeiCoreImage;
  FILE_SECTION_REQ Req = {
    .Ctx = Ctx,
    .FileType = EFI_FV_FILETYPE_PEI_CORE,
    .SectionType = EFI_SECTION_TE, // Expect Terse Executable format
  };

  LOG("Enter GetPeiCoreImage\n");
  PeiCoreImage = GetFileSection(&Req);
  if (PeiCoreImage == NULL) {
    return NULL;
  }

  return PeiCoreImage + sizeof(EFI_COMMON_SECTION_HEADER);
}

EFI_PEI_CORE_ENTRY_POINT
FindPeiCoreEntryPoint(
  IN OUT SEC_CONTEXT *Ctx
  )
{
  VOID *PeiCoreImg;
  VOID *PeiCoreEp;
  EFI_TE_IMAGE_HEADER *Hdr;
  UINT8 *Ep;

  LOG("Enter FindPeiCoreEntryPoint\n");

  PeiCoreImg = GetPeiCoreImage(Ctx);
  if (PeiCoreImg == NULL) {
    LOG("Failed to get PEI core image | line: %d\n", __LINE__);
    return NULL;
  }

  Hdr = (EFI_TE_IMAGE_HEADER *) PeiCoreImg;
  Ep = (UINT8 *) PeiCoreImg;
  Ep -= Hdr->StrippedSize;
  Ep += sizeof(*Hdr) + Hdr->AddressOfEntryPoint & 0xffffffff;

  LOG("DataDirectory ImageBase: 0x%x\n", (UINT32) Hdr->ImageBase);
  LOG("DataDirectory VritualAddress: 0x%x\n", Hdr->DataDirectory[0].VirtualAddress);
  LOG("DataDirectory VritualAddress: 0x%x\n", Hdr->DataDirectory[1].VirtualAddress);

  Ctx->Status = EFI_SUCCESS;
  Ctx->Line = __LINE__;

  return (EFI_PEI_CORE_ENTRY_POINT) Ep;
}

/**
  The entry point of SEC Image.

  See UEFI PI Spec 1.8, "I-17.1 Security (SEC) phase information" for details.

  @return This function should not return.

**/
VOID
SecMain(VOID)
{
  EFI_PEI_CORE_ENTRY_POINT PeiEp;
  EFI_FIRMWARE_VOLUME_HEADER BootFv;
  UINTN SerialRegisterBase;
  SEC_CONTEXT Ctx;
  FV_HEADER BootFvHdr;

  CopyMem(&BootFvHdr, (VOID *) FixedPcdGet32(PcdBootFvBase), sizeof(BootFvHdr));
  CopyMem(&BootFv, (VOID *) &BootFvHdr, BootFvHdr.HeaderLength);

  SerialRegisterBase = SerialPortInitialize();
  LOG("Enter SEC\n");

  LOG("Boot FV addr %p size %u\n", &BootFv, BootFv.HeaderLength);
  if (BootFv.Signature != EFI_FVH_SIGNATURE) {
    LOG("Bad FV signature at 0x%x\n", BootFv.Signature);
    goto halt;
  }

  if (!IsValidBootFv(&BootFv)) {
    LOG("Not a Boot FV at %p\n", &BootFv);
    goto halt;
  }

  LOG("Boot FV at %p\n", &BootFv);
  ZeroMem(&Ctx, sizeof(Ctx));

  Ctx.SecData.DataSize = sizeof(EFI_SEC_PEI_HAND_OFF);
  Ctx.SecData.BootFirmwareVolumeBase = FixedPcdGet32(PcdBootFvBase);
  Ctx.SecData.BootFirmwareVolumeSize = (UINTN) BootFv.FvLength;

  PeiEp = FindPeiCoreEntryPoint(&Ctx);
  if (PeiEp == NULL) {
    LOG("Failed to find PEI core entry point | Status '%a' %u\n",
      StatusToAsciiStr(Ctx.Status), Ctx.Line);
    goto halt;
  }

  Ctx.SecData.TemporaryRamBase = (VOID *) FixedPcdGet32(PcdPeiTemporaryRamBase);
  Ctx.SecData.TemporaryRamSize = (UINTN) FixedPcdGet32(PcdPeiTemporaryRamSize);

  Ctx.SecData.PeiTemporaryRamBase = Ctx.SecData.TemporaryRamBase;
  Ctx.SecData.PeiTemporaryRamSize = Ctx.SecData.TemporaryRamSize >> 1;

  Ctx.SecData.StackBase = Ctx.SecData.TemporaryRamBase;
  Ctx.SecData.StackBase += Ctx.SecData.TemporaryRamSize >> 1;
  Ctx.SecData.StackSize = Ctx.SecData.TemporaryRamSize >> 1;

  LOG("Call PEI core at %p\n", PeiEp);

  PeiEp(&Ctx.SecData, NULL);

  LOG("Fatal error: returned from PEI entry point\n");

halt:
  LOG("-= Boot failed =-\n");
}
