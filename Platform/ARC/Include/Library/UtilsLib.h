/** @file
  Common utilities.

  Copyright (c) 2023 Basemark Oy

  Author: Aliaksei Katovich @ basemark.com

  Released under the GNU General Public License, version 2
**/

#ifndef UTILS_LIB_H_
#define UTILS_LIB_H_

#include <Uefi/UefiBaseType.h>
#include <Library/PrintLib.h>
#include <Library/SerialPortLib.h>

#define MAX_STR_LEN 100

#ifdef VERBOSE
#define LOG(...) {\
  CHAR8 Str_[MAX_STR_LEN];\
  UINTN StrLen_ = AsciiSPrint(Str_, sizeof(Str_), __VA_ARGS__);\
  SerialPortWrite(Str_, StrLen_);\
}
#else
#define LOG(...) ;
#endif

typedef struct {
  CHAR8 Data[sizeof("0xffffffff")];
} INT32_STR;

#define GUID_STR_MAX 36

typedef struct {
  CHAR8 Data[GUID_STR_MAX + 1]; // +1 for '\0'
} GUID_STR;

VOID
ByteToAscii(
  IN OUT CHAR8 Str[2],
  IN UINT8 Byte
  );

VOID
GuidToAsciiStr(
  IN EFI_GUID *EfiGuid,
  OUT GUID_STR *GuidStr
  );

CONST CHAR8 *
StatusToAsciiStr(
  IN EFI_STATUS Status
  );

VOID
Int32ToAscii(
  IN UINT32 Val,
  IN OUT INT32_STR *Str
  );

#endif // UTILS_LIB_H_
