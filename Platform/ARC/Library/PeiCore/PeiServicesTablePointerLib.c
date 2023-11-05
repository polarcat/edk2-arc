/** @file
  Provides a service to access PEI services table.

  Copyright (c) 2023 Basemark Oy

  Author: Aliaksei Katovich @ basemark.com

  Released under the BSD-2-Clause License
**/

#include <Library/UtilsLib.h>
#include <Library/BaseLib.h>

STATIC CONST EFI_PEI_SERVICES **mPeiServicesTablePointer;

/**
  Get PEI services table pointer.

  @return The pointer to PEI services.

**/
CONST EFI_PEI_SERVICES **
EFIAPI
GetPeiServicesTablePointer(
  VOID
  )
{
  LOG("Get PEI services table at %p\n", mPeiServicesTablePointer);
  return mPeiServicesTablePointer;
}

/**
  Cache PEI services table pointer.

  @param  PeiServicesTablePointer pointer to PEI services table.
**/
VOID
EFIAPI
SetPeiServicesTablePointer(
  IN CONST EFI_PEI_SERVICES  **PeiServicesTablePointer
  )
{
  LOG("Set PEI service table at %p\n", PeiServicesTablePointer);
  mPeiServicesTablePointer = PeiServicesTablePointer;
}

/**
  Perform CPU specific actions required to migrate the PEI Services Table
  pointer from temporary RAM to permanent RAM.

**/
VOID
EFIAPI
MigratePeiServicesTablePointer(VOID)
{
  LOG_ENTER();
}
