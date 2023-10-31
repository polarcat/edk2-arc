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
  PeCoffLib | MdePkg/Library/BasePeCoffLib/BasePeCoffLib.inf
  PeCoffGetEntryPointLib | MdePkg/Library/BasePeCoffGetEntryPointLib/BasePeCoffGetEntryPointLib.inf
  PeCoffExtraActionLib | MdePkg/Library/BasePeCoffExtraActionLibNull/BasePeCoffExtraActionLibNull.inf
  PcdLib | MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  RegisterFilterLib | MdePkg/Library/RegisterFilterLibNull/RegisterFilterLibNull.inf
  IoLib | MdePkg/Library/BaseIoLibIntrinsic/BaseIoLibIntrinsic.inf
  SerialPortLib | Platform/ARC/Library/SerialPortLib/Ns16550.inf
  UtilsLib | Platform/ARC/Library/UtilsLib/UtilsLib.inf

[LibraryClasses.common.PEI_CORE]
  ReportStatusCodeLib | MdePkg/Library/BaseReportStatusCodeLibNull/BaseReportStatusCodeLibNull.inf
  PeiServicesLib | MdePkg/Library/PeiServicesLib/PeiServicesLib.inf
  PerformanceLib | MdePkg/Library/BasePerformanceLibNull/BasePerformanceLibNull.inf
  #PerformanceLib | MdeModulePkg/Library/PeiPerformanceLib/PeiPerformanceLib.inf
  HobLib | MdePkg/Library/PeiHobLib/PeiHobLib.inf
  PeiCoreEntryPoint | MdePkg/Library/PeiCoreEntryPoint/PeiCoreEntryPoint.inf
  MemoryAllocationLib | MdePkg/Library/PeiMemoryAllocationLib/PeiMemoryAllocationLib.inf
  PeiServicesTablePointerLib | MdePkg/Library/PeiServicesTablePointerLib/PeiServicesTablePointerLib.inf
  CacheMaintenanceLib | Platform/ARC/Library/CpuLib/CacheMaintenanceLib.inf
  CpuLib | Platform/ARC/Library/CpuLib/CpuLib.inf

[Components.common.SEC]
  Platform/ARC/Library/Sec/SecMain.inf

[Components.common.PEI_CORE]
  Platform/ARC/Library/PeiCore/PeiCore.inf
