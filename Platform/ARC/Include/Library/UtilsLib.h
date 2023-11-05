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
#include <Pi/PiFirmwareVolume.h>
#include <Pi/PiFirmwareFile.h>
#include <Guid/AprioriFileName.h>

#define EFI_PEI_PPI_DESCRIPTOR_PPI_PIC \
  (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_PIC)

#define MAX_STR_LEN 256

#ifdef VERBOSE
#define LOG(...) {\
  CHAR8 Str_[MAX_STR_LEN];\
  UINT8 StrLen_ = AsciiSPrint(Str_, sizeof(Str_), __VA_ARGS__);\
  SerialPortWrite(Str_, StrLen_);\
}
#define LOG_ENTER() {\
  CHAR8 Str_[MAX_STR_LEN];\
  UINT8 StrLen_ = AsciiSPrint(Str_, sizeof(Str_), "Enter %a:%d | %a\n",\
    __func__, __LINE__, __FILE__);\
  SerialPortWrite(Str_, StrLen_);\
}
#define LOG_EXIT() {\
  CHAR8 Str_[MAX_STR_LEN];\
  UINT8 StrLen_ = AsciiSPrint(Str_, sizeof(Str_), "Exit %a:%d | %a\n",\
    __func__, __LINE__, __FILE__);\
  SerialPortWrite(Str_, StrLen_);\
}
#else
#define LOG(...) ;
#define LOG_ENTER() ;
#define LOG_EXIT() ;
#endif

typedef struct {
  EFI_STATUS Status;
  UINT32 SourceCodeLine;
} STATUS_INFO;

#define SET_STATUS_INFO(StatusInfoPtr, Val) {\
  if ((StatusInfoPtr) != NULL) {\
    (StatusInfoPtr)->Status = Val;\
    (StatusInfoPtr)->SourceCodeLine = __LINE__;\
  }\
}

typedef struct {
  CHAR8 Data[sizeof("0xffffffff")];
} INT32_STR;

#define GUID_STR_MAX 36

typedef struct {
  CHAR8 Data[GUID_STR_MAX + 1]; // +1 for '\0'
} GUID_STR;

inline
EFI_PHYSICAL_ADDRESS
ToPhysAddr(
  IN VOID *Ptr
  )
{
  return (EFI_PHYSICAL_ADDRESS) (UINTN) Ptr;
}

VOID
ByteToAscii(
  IN UINT8 Byte,
  IN OUT CHAR8 Str[2]
  );

VOID
GuidToAsciiStr(
  IN CONST EFI_GUID *EfiGuid,
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

BOOLEAN CompareGuids(
  IN CONST EFI_GUID *Guid1,
  IN CONST EFI_GUID *Guid2
  );

VOID *
GetFileSection(
  IN VOID *FvBase,
  IN EFI_SECTION_TYPE SectionType,
  IN EFI_FV_FILETYPE FileType,
  IN CONST EFI_GUID *FileName OPTIONAL,
  OUT STATUS_INFO *StatusInfo OPTIONAL
  );

VOID *
GetTeEntryPoint(
  IN VOID *FvBase,
  IN EFI_FV_FILETYPE FileType,
  IN CONST EFI_GUID *FileName OPTIONAL,
  OUT STATUS_INFO *StatusInfo OPTIONAL
  );

PEI_APRIORI_FILE_CONTENTS *
GetAprioriFile(
  IN VOID *FvBase,
  OUT STATUS_INFO *StatusInfo OPTIONAL
  );

#endif // UTILS_LIB_H_
