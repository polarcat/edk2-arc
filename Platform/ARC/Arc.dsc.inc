[Defines]
  PLATFORM_GUID = 5d8fc9d8-6a53-4cb6-b30e-2169c1118d27
  PLATFORM_VERSION = 0.1
  DSC_SPECIFICATION = 0x00010005
  OUTPUT_DIRECTORY = Build/$(PLATFORM_NAME)
  SUPPORTED_ARCHITECTURES = ARC2
  BUILD_TARGETS = DEBUG|RELEASE|NOOPT
  SKUID_IDENTIFIER = DEFAULT

[LibraryClasses.common]
  BaseLib | MdePkg/Library/BaseLib/BaseLib.inf
  BaseMemoryLib | MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
  DebugLib|MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
  PrintLib | MdePkg/Library/BasePrintLib/BasePrintLib.inf
  PcdLib | MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  RegisterFilterLib | MdePkg/Library/RegisterFilterLibNull/RegisterFilterLibNull.inf
  IoLib | MdePkg/Library/BaseIoLibIntrinsic/BaseIoLibIntrinsic.inf
  SerialPortLib | Platform/ARC/Library/SerialPortLib/Ns16550.inf
  UtilsLib | Platform/ARC/Library/UtilsLib/UtilsLib.inf

[LibraryClasses.common.PEI_CORE]
  PeiServicesTablePointerLib | MdePkg/Library/PeiServicesTablePointerLib/PeiServicesTablePointerLib.inf
  PeiCoreEntryPoint | MdePkg/Library/PeiCoreEntryPoint/PeiCoreEntryPoint.inf
#  HobLib | MdePkg/Library/PeiHobLib/PeiHobLib.inf

  CacheMaintenanceLib | Platform/ARC/Library/CpuLib/CpuCache.inf
  CpuLib | Platform/ARC/Library/CpuLib/CpuLib.inf

[LibraryClasses.common.PEIM]
  PeiServicesTablePointerLib | Platform/ARC/Library/PeiCore/PeiServicesTablePointerLib.inf
  PeimEntryPoint | MdePkg/Library/PeimEntryPoint/PeimEntryPoint.inf
  PeiServicesLib | MdePkg/Library/PeiServicesLib/PeiServicesLib.inf

[LibraryClasses.common.DXE_CORE]
  HobLib | MdePkg/Library/DxeCoreHobLib/DxeCoreHobLib.inf
  DxeCoreEntryPoint | MdePkg/Library/DxeCoreEntryPoint/DxeCoreEntryPoint.inf
  MemoryAllocationLib | MdeModulePkg/Library/DxeCoreMemoryAllocationLib/DxeCoreMemoryAllocationLib.inf
  UefiDecompressLib | MdePkg/Library/BaseUefiDecompressLib/BaseUefiDecompressLib.inf
  PerformanceLib | MdePkg/Library/BasePerformanceLibNull/BasePerformanceLibNull.inf
  UefiLib | MdePkg/Library/UefiLib/UefiLib.inf
  PeCoffLib | MdePkg/Library/BasePeCoffLib/BasePeCoffLib.inf
  PeCoffGetEntryPointLib | MdePkg/Library/BasePeCoffGetEntryPointLib/BasePeCoffGetEntryPointLib.inf
  PeCoffExtraActionLib | MdePkg/Library/BasePeCoffExtraActionLibNull/BasePeCoffExtraActionLibNull.inf
  ExtractGuidedSectionLib | MdePkg/Library/DxeExtractGuidedSectionLib/DxeExtractGuidedSectionLib.inf
  DevicePathLib | MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
  UefiRuntimeServicesTableLib | MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
  UefiBootServicesTableLib | MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  ReportStatusCodeLib | MdePkg/Library/BaseReportStatusCodeLibNull/BaseReportStatusCodeLibNull.inf
  DxeServicesLib | MdePkg/Library/DxeServicesLib/DxeServicesLib.inf
  DebugAgentLib | MdeModulePkg/Library/DebugAgentLibNull/DebugAgentLibNull.inf
  LocalApicLib | UefiCpuPkg/Library/BaseXApicLib/BaseXApicLib.inf
  CcExitLib | UefiCpuPkg/Library/CcExitLibNull/CcExitLibNull.inf
  TimerLib | MdePkg/Library/BaseTimerLibNullTemplate/BaseTimerLibNullTemplate.inf

  CacheMaintenanceLib | Platform/ARC/Library/CpuLib/CpuCache.inf
  CpuExceptionHandlerLib | Platform/ARC/Library/CpuLib/CpuException.inf
  SynchronizationLib | Platform/ARC/Library/CpuLib/CpuSync.inf
  CpuLib | Platform/ARC/Library/CpuLib/CpuLib.inf

[Components]
  # PEI
  Platform/ARC/Library/Sec/SecMain.inf
  Platform/ARC/Library/PeiCore/PeiCore.inf
  Platform/ARC/Library/PeiCore/DxeIpl.inf

  # DXE
  MdeModulePkg/Core/Dxe/DxeMain.inf
