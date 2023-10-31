/** @file
  Common utilities.

  Copyright (c) 2023 Basemark Oy

  Author: Aliaksei Katovich @ basemark.com

  Released under the BSD-2-Clause License
**/

#include <UtilsLib.h>

VOID
ByteToAscii(
  IN OUT CHAR8 Str[2],
  IN UINT8 Byte
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
  ByteToAscii(&Str->Data[3], (Val >> 24) & 0xff);
  ByteToAscii(&Str->Data[5], (Val >> 16) & 0xff);
  ByteToAscii(&Str->Data[7], (Val >> 8) & 0xff);
  ByteToAscii(&Str->Data[9], Val & 0xff);
  Str->Data[11] = '\0';
}

VOID
GuidToAsciiStr(
  IN EFI_GUID *EfiGuid,
  OUT GUID_STR *GuidStr
  )
{
  UINT8 Idx1;
  UINT8 Idx2;

  ByteToAscii(&GuidStr->Data[0], (EfiGuid->Data1 >> 24) & 0xff);
  ByteToAscii(&GuidStr->Data[2], (EfiGuid->Data1 >> 16) & 0xff);
  ByteToAscii(&GuidStr->Data[4], (EfiGuid->Data1 >> 8) & 0xff);
  ByteToAscii(&GuidStr->Data[6], EfiGuid->Data1 & 0xff);
  GuidStr->Data[8] = '-';

  ByteToAscii(&GuidStr->Data[9], (EfiGuid->Data2 >> 8) & 0xff);
  ByteToAscii(&GuidStr->Data[11], EfiGuid->Data2 & 0xff);
  GuidStr->Data[13] = '-';

  ByteToAscii(&GuidStr->Data[14], (EfiGuid->Data3 >> 8) & 0xff);
  ByteToAscii(&GuidStr->Data[16], EfiGuid->Data3 & 0xff);
  GuidStr->Data[18] = '-';

  ByteToAscii(&GuidStr->Data[19], EfiGuid->Data4[0] & 0xff);
  ByteToAscii(&GuidStr->Data[21], EfiGuid->Data4[1] & 0xff);
  GuidStr->Data[23] = '-';

  for (Idx1 = 2, Idx2 = 24; Idx2 < GUID_STR_MAX; Idx1++) {
    ByteToAscii(&GuidStr->Data[Idx2], EfiGuid->Data4[Idx1]);
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
