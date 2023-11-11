/** @file
  Common utilities.

  Copyright (c) 2023 Basemark Oy

  Author: Aliaksei Katovich @ basemark.com

  Released under the BSD-2-Clause License
**/

#ifdef VERBOSE_FV_IO
#undef VERBOSE
#endif

#include <UtilsLib.h>
#include <IndustryStandard/PeImage.h>

VOID
ByteToAscii(
  IN UINT8 Byte,
  IN OUT CHAR8 Str[2]
  )
{
  UINT8 Val;

  Val = (Byte >> 4) & 0x0f;
  if (Val >= 0 && Val <= 9) {
    *Str = '0' + Val;
  } else if (Val >= 10 && Val <= 15) {
    *Str = 'a' + Val - 10;
  } else {
    *Str = '\0';
  }

  Val = Byte & 0x0f;
  if (Val >= 0 && Val <= 9) {
    *(Str + 1) = '0' + Val;
  } else if (Val >= 10 && Val <= 15) {
    *(Str + 1) = 'a' + Val - 10;
  } else {
    *Str = '\0';
  }
}

VOID
Int32ToAscii(
  IN UINT32 Val,
  IN OUT INT32_STR *Str
  )
{
  Str->Data[0] = '0';
  Str->Data[1] = 'x';
  ByteToAscii((Val >> 24) & 0xff, &Str->Data[3]);
  ByteToAscii((Val >> 16) & 0xff, &Str->Data[5]);
  ByteToAscii((Val >> 8) & 0xff, &Str->Data[7]);
  ByteToAscii(Val & 0xff, &Str->Data[9]);
  Str->Data[11] = '\0';
}

VOID
GuidToAsciiStr(
  IN CONST EFI_GUID *EfiGuid,
  OUT GUID_STR *GuidStr
  )
{
  UINT8 Idx1;
  UINT8 Idx2;

  ByteToAscii((EfiGuid->Data1 >> 24) & 0xff, &GuidStr->Data[0]);
  ByteToAscii((EfiGuid->Data1 >> 16) & 0xff, &GuidStr->Data[2]);
  ByteToAscii((EfiGuid->Data1 >> 8) & 0xff, &GuidStr->Data[4]);
  ByteToAscii(EfiGuid->Data1 & 0xff, &GuidStr->Data[6]);
  GuidStr->Data[8] = '-';

  ByteToAscii((EfiGuid->Data2 >> 8) & 0xff, &GuidStr->Data[9]);
  ByteToAscii(EfiGuid->Data2 & 0xff, &GuidStr->Data[11]);
  GuidStr->Data[13] = '-';

  ByteToAscii((EfiGuid->Data3 >> 8) & 0xff, &GuidStr->Data[14]);
  ByteToAscii(EfiGuid->Data3 & 0xff, &GuidStr->Data[16]);
  GuidStr->Data[18] = '-';

  ByteToAscii(EfiGuid->Data4[0] & 0xff, &GuidStr->Data[19]);
  ByteToAscii(EfiGuid->Data4[1] & 0xff, &GuidStr->Data[21]);
  GuidStr->Data[23] = '-';

  for (Idx1 = 2, Idx2 = 24; Idx2 < GUID_STR_MAX; Idx1++) {
    ByteToAscii(EfiGuid->Data4[Idx1], &GuidStr->Data[Idx2]);
    Idx2 += 2;
  }

  GuidStr->Data[GUID_STR_MAX] = '\0';
}

#define IF_STATUS_EQ(val, code) if (val == code) { return #code; }
#define ELSE_IF_STATUS_EQ(val, code) else if (val == code) { return #code; }
#define ELSE_STATUS(code) else { return #code; }

CONST CHAR8 *
StatusToAsciiStr(
  IN EFI_STATUS Status
  )
{
  IF_STATUS_EQ(Status, EFI_SUCCESS)
  ELSE_IF_STATUS_EQ(Status, EFI_LOAD_ERROR)
  ELSE_IF_STATUS_EQ(Status, EFI_INVALID_PARAMETER)
  ELSE_IF_STATUS_EQ(Status, EFI_UNSUPPORTED)
  ELSE_IF_STATUS_EQ(Status, EFI_BAD_BUFFER_SIZE)
  ELSE_IF_STATUS_EQ(Status, EFI_BUFFER_TOO_SMALL)
  ELSE_IF_STATUS_EQ(Status, EFI_NOT_READY)
  ELSE_IF_STATUS_EQ(Status, EFI_DEVICE_ERROR)
  ELSE_IF_STATUS_EQ(Status, EFI_WRITE_PROTECTED)
  ELSE_IF_STATUS_EQ(Status, EFI_OUT_OF_RESOURCES)
  ELSE_IF_STATUS_EQ(Status, EFI_VOLUME_CORRUPTED)
  ELSE_IF_STATUS_EQ(Status, EFI_VOLUME_FULL)
  ELSE_IF_STATUS_EQ(Status, EFI_NO_MEDIA)
  ELSE_IF_STATUS_EQ(Status, EFI_MEDIA_CHANGED)
  ELSE_IF_STATUS_EQ(Status, EFI_NOT_FOUND)
  ELSE_IF_STATUS_EQ(Status, EFI_ACCESS_DENIED)
  ELSE_IF_STATUS_EQ(Status, EFI_NO_RESPONSE)
  ELSE_IF_STATUS_EQ(Status, EFI_NO_MAPPING)
  ELSE_IF_STATUS_EQ(Status, EFI_TIMEOUT)
  ELSE_IF_STATUS_EQ(Status, EFI_NOT_STARTED)
  ELSE_IF_STATUS_EQ(Status, EFI_ALREADY_STARTED)
  ELSE_IF_STATUS_EQ(Status, EFI_ABORTED)
  ELSE_IF_STATUS_EQ(Status, EFI_ICMP_ERROR)
  ELSE_IF_STATUS_EQ(Status, EFI_TFTP_ERROR)
  ELSE_IF_STATUS_EQ(Status, EFI_PROTOCOL_ERROR)
  ELSE_IF_STATUS_EQ(Status, EFI_INCOMPATIBLE_VERSION)
  ELSE_IF_STATUS_EQ(Status, EFI_SECURITY_VIOLATION)
  ELSE_IF_STATUS_EQ(Status, EFI_CRC_ERROR)
  ELSE_IF_STATUS_EQ(Status, EFI_END_OF_MEDIA)
  ELSE_IF_STATUS_EQ(Status, EFI_END_OF_FILE)
  ELSE_STATUS("UNKNOWN STATUS")
}

BOOLEAN CompareGuids(
  IN CONST EFI_GUID *Guid1,
  IN CONST EFI_GUID *Guid2
  )
{
    return ((UINT64 *) Guid1)[0] == ((UINT64 *) Guid2)[0] &&
      ((UINT64 *) Guid1)[1] == ((UINT64 *) Guid2)[1];
}

VOID *
ToVoidPtr(
  IN EFI_PHYSICAL_ADDRESS Addr
  )
{
  return (VOID *) (UINTN) Addr;
}

VOID *
FindSection(
  IN EFI_SECTION_TYPE SectionType,
  IN EFI_PHYSICAL_ADDRESS SectionsAddr,
  IN EFI_PHYSICAL_ADDRESS SectionsEnd,
  OUT STATUS_INFO *StatusInfo OPTIONAL
  )
{
  EFI_PHYSICAL_ADDRESS Addr; // Address iterator
  EFI_COMMON_SECTION_HEADER *Section;
  UINT32 Size;

  Addr = AlignAddr(SectionsAddr, 4);

  while (Addr < SectionsEnd) {
    Section = (EFI_COMMON_SECTION_HEADER *) (UINTN) Addr;
    Size = SECTION_SIZE(Section);
    DBG("| Section %p size 0x%x type 0x%x\n", Section, Size, Section->Type);
    if (Size < sizeof(*Section)) {
      SET_STATUS_INFO(StatusInfo, EFI_VOLUME_CORRUPTED);
      return NULL;
    }

    if (Addr + Size > SectionsEnd) {
      SET_STATUS_INFO(StatusInfo, EFI_VOLUME_CORRUPTED);
      return NULL;
    }

    if (Section->Type == SectionType) {
        SET_STATUS_INFO(StatusInfo, EFI_SUCCESS);
        return Section;
    }

    Addr += Size;
  }

  SET_STATUS_INFO(StatusInfo, EFI_NOT_FOUND);
  return NULL;
}

VOID *
GetFileSection(
  IN VOID *FvBase,
  IN EFI_SECTION_TYPE SectionType,
  IN EFI_FV_FILETYPE FileType,
  IN CONST EFI_GUID *FileName OPTIONAL,
  OUT STATUS_INFO *StatusInfo OPTIONAL
  )
{
  EFI_FIRMWARE_VOLUME_HEADER *Fv;
  EFI_PHYSICAL_ADDRESS Addr; // Address iterator
  EFI_PHYSICAL_ADDRESS Eov; // End of volume
  EFI_PHYSICAL_ADDRESS Eof; // End of file
  EFI_FFS_FILE_HEADER *File;
  GUID_STR GuidStr;
  VOID *Image;
  BOOLEAN FileNameOk;

  Fv = (EFI_FIRMWARE_VOLUME_HEADER *) FvBase;
  Eov = ToPhysAddr(Fv) + Fv->FvLength;
  Addr = AlignAddr(ToPhysAddr(Fv) + Fv->HeaderLength, 8);

  while (Addr < Eov) {
    File = (EFI_FFS_FILE_HEADER *) (UINTN) Addr;
    Eof = Addr + FFS_FILE_SIZE(File);

    GuidToAsciiStr(&File->Name, &GuidStr);
    DBG("| Check file at %p type 0x%x name %a\n", File, File->Type, GuidStr.Data);

    if (Eof > Eov) { // Sanity check
      SET_STATUS_INFO(StatusInfo, EFI_VOLUME_CORRUPTED);
      return NULL;
    }

    if (!FileName) {
      FileNameOk = TRUE;
    } else {
      FileNameOk = CompareGuids(FileName, &File->Name);
    }

    if (FileNameOk && File->Type == FileType) {
      return FindSection(SectionType, ToPhysAddr(File + 1), Eof, StatusInfo);
    }

    Addr = AlignAddr(Eof, 8);
  }

  SET_STATUS_INFO(StatusInfo, EFI_NOT_FOUND);
  return NULL;
}

VOID *
GetTeEntryPoint(
  IN VOID *FvBase,
  IN EFI_FV_FILETYPE FileType,
  IN CONST EFI_GUID *FileName OPTIONAL,
  OUT STATUS_INFO *StatusInfo OPTIONAL
  )
{
  VOID *Ptr;
  EFI_TE_IMAGE_HEADER *TeHdr;

  Ptr = GetFileSection(FvBase, EFI_SECTION_TE, FileType, FileName, StatusInfo);
  if (Ptr == NULL) {
    return NULL;
  } else {
    Ptr += sizeof(EFI_COMMON_SECTION_HEADER);
    TeHdr = (EFI_TE_IMAGE_HEADER *) Ptr;
    Ptr -= TeHdr->StrippedSize;
    Ptr += sizeof(*TeHdr) + TeHdr->AddressOfEntryPoint & 0xffffffff;
    return Ptr;
  }
}

PEI_APRIORI_FILE_CONTENTS *
GetAprioriFile(
  IN VOID *FvBase,
  OUT STATUS_INFO *StatusInfo OPTIONAL
  )
{
  VOID *Ptr;
  // PEI_APRIORI_FILE_NAME_GUID
  CONST EFI_GUID FileName = {
    0x1b45cc0a, 0x156a, 0x428a, {
      0xaf, 0x62, 0x49, 0x86, 0x4d, 0xa0, 0xe6, 0xe6
    }
  };

  Ptr = GetFileSection(FvBase, EFI_SECTION_RAW, EFI_FV_FILETYPE_FREEFORM,
    &FileName, StatusInfo);
  if (Ptr != NULL) {
    Ptr += sizeof(EFI_COMMON_SECTION_HEADER);
  }

  return Ptr;
}
