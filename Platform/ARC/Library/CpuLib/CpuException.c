/** @file
  ARC CPU exceptions implementation.

  See MdeModulePkg/Include/Library/CpuExceptionHandlerLib.h for detailed
  description of each function.

  Copyright (c) 2023 Basemark Oy

  Author: Aliaksei Katovich @ basemark.com

  Released under the BSD-2-Clause License
**/

#include <Ppi/VectorHandoffInfo.h>

EFI_STATUS
EFIAPI
InitializeCpuExceptionHandlers(
  IN EFI_VECTOR_HANDOFF_INFO  *VectorInfo OPTIONAL
  )
{
  return EFI_UNSUPPORTED;
}

EFI_STATUS
EFIAPI
InitializeSeparateExceptionStacks(
  IN     VOID   *Buffer,
  IN OUT UINTN  *BufferSize
  )
{
  return EFI_UNSUPPORTED;
}
