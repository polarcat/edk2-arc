/** @file
  PEI core entry point helpers.

  Copyright (c) 2023 Basemark Oy

  Author: Aliaksei Katovich @ basemark.com

  Released under the BSD-2-Clause License
**/

#include <Core/Pei/PeiMain.h>

#define MAX_CORE_FV 2

typedef struct {
  EFI_PEI_SERVICES    *PsPtr;
  EFI_PEI_SERVICES    Ps;
  PEI_PPI_DATABASE    PpiData;
  PEI_CORE_FV_HANDLE  Fv[MAX_CORE_FV];
} PEI_CORE_CONTEXT;

#define PS_TO_PEI_CONTEXT_PTR(PsPtr_) BASE_CR(PsPtr_, PEI_CORE_CONTEXT, PsPtr)

PEI_CORE_CONTEXT *
GetCorePeiInstance(
  IN CONST EFI_PEI_SERVICES **PeiServices
  );
