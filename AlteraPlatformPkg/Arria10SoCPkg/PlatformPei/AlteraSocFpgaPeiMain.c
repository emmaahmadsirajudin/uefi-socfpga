/** @file

  Copyright (c) 2015, Altera Corporation. All rights reserved.

  Redistribution and use in source and binary forms, with or without modification,
  are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

  2. Redistributions in binary form must reproduce the above copyright notice, this
  list of conditions and the following disclaimer in the documentation and/or other
  materials provided with the distribution.

  3. Neither the name of the copyright holder nor the names of its contributors may
  be used to endorse or promote products derived from this software without specific
  prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
  SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
  DAMAGE.

**/
#include <Ppi/ArmMpCoreInfo.h>
#include <Ppi/GuidedSectionExtraction.h>

#include <Guid/ArmGlobalVariableHob.h>
#include <Guid/LzmaDecompress.h>

#include <AlteraPlatform.h>
#include <PiPei.h>
#include <Library/ArmLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugAgentLib.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/PeCoffGetEntryPointLib.h>
#include <Library/PerformanceLib.h>
#include <Library/PrePiHobListPointerLib.h>
#include <Library/PrePiLib.h>
#include <Library/PrintLib.h>
#include <Library/SerialPortLib.h>
#include <Library/SerialPortPrintLib.h>
#include <Library/TimerLib.h>

#include "AlteraSocFpgaPeiMain.h"
#include "Assert.h"
#include "Banner.h"
#include "Boot.h"
#include "LzmaDecompress.h"
#include "MemoryController.h"
#include "PlatformInit.h"
#include "ResetManager.h"
#include "SdMmc.h"
#include "SecurityManager.h"
#include "SystemManager.h"

#define EFI_DXE_CORE_GUID \
{ \
  0xD6A2CB7F, 0x6A18, 0x4e2f, {0xB4, 0x3B, 0x99, 0x20, 0xa7, 0x33, 0x70, 0x0a} \
}

typedef
VOID
(EFIAPI *DXE_CORE_ENTRY_POINT) (
  IN  VOID *HobStart
  );

VOID
EFIAPI
AlteraSocFpgaPeiMainEntry (
  IN CONST EFI_SEC_PEI_HAND_OFF        *SecCoreData,
  IN CONST EFI_PEI_PPI_DESCRIPTOR      *PpiList,
  IN VOID                              *Data
  )
{
  // Initialize the Timer Library
  TimerConstructor ();

  // Init Hob and also record time of PEI start
  InitializeHOBs (SecCoreData, PpiList, GetPerformanceCounter());

  // Initialize Memory Serial Log
  MemorySerialLogInit ();

  // Print some useful information to semihosting console if enabled
  PrintModuleEntryPointAndMemoryMapInfoToMemorySerialLogOrSemihostingConsoleIfEnabled (SecCoreData);

  // Initialize Platform
  PeiStagePlatformInit();

  // External Memory should be up, build the System Memory Hobs
  BuildSystemMemoryHOBs (
    GetMpuWindowDramBaseAddr(),
    GetMpuWindowDramSize()
    );

  // Boot UEFI DXE phase
  BootCompressedDxeFv ();    // Try LZMA compressed DXE FV
  BootUnCompressedDxeFv ();  // If it fail try RAW DXE FV

  // Should not reach here
  ASSERT_PLATFORM_INIT(0);
}

VOID
EFIAPI
MemorySerialLogInit (
  VOID
  )
{
  // At this point, UART is not up yet, so the primary purpose of calling
  // this DisplayFirmwareVersion function is to give a header to MemorySerialLogWrite
  DisplayFirmwareVersion ();

  // Display System Manager Info
  DisplaySystemManagerInfo ();

  // Display Reset Manager Info
  DisplayResetManagerInfo ();

  // Display Security Manager Info
  DisplaySecurityManagerInfo ();
}


VOID
EFIAPI
PrintModuleEntryPointAndMemoryMapInfoToMemorySerialLogOrSemihostingConsoleIfEnabled (
  IN CONST EFI_SEC_PEI_HAND_OFF        *SecCoreData
  )
{
  UINT32 ArmPlatformSecEntry;
  UINT32 ArmPlatformPrePeiCoreEntry;
  UINT32 AlteraSocFpgaPeiMainEntry;
  UINT32 SecFvBegin;
  UINT32 SecFvEnd;
  UINT32 PeiFvBegin;
  UINT32 PeiFvEnd;
  UINT32 DtbFvBegin;
  UINT32 DtbFvEnd;
  UINT32 MkpFvBegin;
  UINT32 MkpFvEnd;
  UINT32 MemLogBegin;
  UINT32 MemLogEnd;
  UINT32 TotalPeiMemBegin;
  UINT32 TotalPeiMemEnd;
  UINT32 PpiListBegin;
  UINT32 PpiListEnd;
  UINT32 PeiHeapBegin;
  UINT32 PeiHeapEnd;
  UINT32 PeiStackBegin;
  UINT32 PeiStackEnd;
  UINT32 GlobalVariablePtrBegin;
  UINT32 GlobalVariablePtrEnd;
  UINT32 SecStackBegin;
  UINT32 SecStackEnd;
  UINT32 MonModeStackBegin;
  UINT32 MonModeStackEnd;

  // Use OCRAM base as SEC entry
  ArmPlatformSecEntry = ALT_OCRAM_OFST;
  // Decode the entry offset from BL jump opcode
  ArmPlatformPrePeiCoreEntry = PcdGet32(PcdFvBaseAddress) + (((*((UINT32 *)PcdGet32(PcdFvBaseAddress)) & 0x00FFFFFF) + 2) * 4);
  // Decode the PEI MAIN entry value patched into this location by GenFv tool's UpdateArmResetVectorIfNeeded function
  AlteraSocFpgaPeiMainEntry = (*((UINT32 *)(PcdGet32(PcdFvBaseAddress) + 4)) & 0xFFFFFFFE);

  SecFvBegin = ALT_OCRAM_OFST;
  SecFvEnd = (UINT32)SecCoreData->BootFirmwareVolumeBase - 1;
  PeiFvBegin = (UINT32)SecCoreData->BootFirmwareVolumeBase;
  PeiFvEnd = (UINT32)SecCoreData->BootFirmwareVolumeBase + (UINT32)SecCoreData->BootFirmwareVolumeSize - 1;
  DtbFvBegin = PcdGet32(PcdFvDtbBaseAddress);
  DtbFvEnd = PcdGet32(PcdFvDtbBaseAddress) + PcdGet32(PcdDtbFvSize) - 1;
  MkpFvBegin = PcdGet32(PcdFvDtbBaseAddress) + PcdGet32(PcdDtbFvSize);
  MkpFvEnd = PcdGet32(PcdFvDtbBaseAddress) + PcdGet32(PcdDtbFvSize) + 4 - 1;
  MemLogBegin = (UINT32)PcdGet64 (PcdMemorySerialLogBase);
  MemLogEnd = (UINT32)PcdGet64 (PcdMemorySerialLogBase) + (UINT32)PcdGet64 (PcdMemorySerialLogSize) - 1;

  TotalPeiMemBegin = PcdGet32 (PcdCPUCoresStackBase);
  TotalPeiMemEnd = PcdGet32 (PcdCPUCoresStackBase) + PcdGet32 (PcdCPUCorePrimaryStackSize) - 1;
  PpiListBegin = PcdGet32 (PcdCPUCoresStackBase);
  PpiListEnd = PcdGet32 (PcdCPUCoresStackBase) + (UINT32)SecCoreData->TemporaryRamBase - PcdGet32 (PcdCPUCoresStackBase) - 1;
  PeiHeapBegin = (UINT32)SecCoreData->PeiTemporaryRamBase;
  PeiHeapEnd = (UINT32)SecCoreData->PeiTemporaryRamBase + (UINT32)SecCoreData->PeiTemporaryRamSize - 1;
  PeiStackBegin = (UINT32)SecCoreData->StackBase;
  PeiStackEnd = (UINT32)SecCoreData->StackBase + (UINT32)SecCoreData->StackSize - 1;
  GlobalVariablePtrBegin = PcdGet32 (PcdCPUCoresStackBase) + PcdGet32 (PcdCPUCorePrimaryStackSize) - PcdGet32 (PcdPeiGlobalVariableSize);
  GlobalVariablePtrEnd = PcdGet32 (PcdCPUCoresStackBase) +  PcdGet32 (PcdCPUCorePrimaryStackSize) - 1;
  SecStackBegin = PcdGet32 (PcdCPUCoresSecStackBase);
  SecStackEnd = PcdGet32 (PcdCPUCoresSecStackBase) + PcdGet32 (PcdCPUCoreSecPrimaryStackSize) - 1;
  MonModeStackBegin = PcdGet32 (PcdCPUCoresSecMonStackBase);
  MonModeStackEnd = PcdGet32 (PcdCPUCoresSecMonStackBase) + PcdGet32 (PcdCPUCoreSecMonStackSize) - 1;

  // Visible only on Memory Serial Log or Semihosting console when enabled

  // Print UEFI Entry Points Map
  SerialPortPrint ("UEFI Entry Points Map: \r\n"
                   "  ArmPlatformSec        Entry = 0x%08x \r\n"
                   "  ArmPlatformPrePeiCore Entry = 0x%08x \r\n"
                   "  AlteraSocFpgaPeiMain  Entry = 0x%08x \r\n",
                    ArmPlatformSecEntry,
                    ArmPlatformPrePeiCoreEntry,
                    AlteraSocFpgaPeiMainEntry);


  // Print OCRAM Memory Map
  SerialPortPrint ("OCRAM Memory Map : \r\n"
                   "SEC Firmware Volume  = 0x%08x - 0x%08x\r\n"
                   "PEI Firmware Volume  = 0x%08x - 0x%08x\r\n"
                   "DTB Firmware Volume  = 0x%08x - 0x%08x\r\n"
                   "MkpImage CheckSum    = 0x%08x - 0x%08x\r\n"
                   "Memory Serial Log    = 0x%08x - 0x%08x \r\n",
                    SecFvBegin,  SecFvEnd,
                    PeiFvBegin,  PeiFvEnd,
                    DtbFvBegin,  DtbFvEnd,
                    MkpFvBegin,  MkpFvEnd,
                   MemLogBegin, MemLogEnd);

  // Print Memory Map for PEI Heap and Stack structure
  SerialPortPrint ("PEI Phase Memory Map: (0x%08x - 0x%08x)\r\n"
                   "  Ppi List           = 0x%08x - 0x%08x\r\n"
                   "  PEI Heap           = 0x%08x - 0x%08x\r\n"
                   "  PEI Stack          = 0x%08x - 0x%08x\r\n"
                   "  GlobalVariablePtr  = 0x%08x - 0x%08x\r\n"
                   "SEC Phase Stack      = 0x%08x - 0x%08x\r\n"
                   "Monitor Mode Stack   = 0x%08x - 0x%08x\r\n",
                    TotalPeiMemBegin, TotalPeiMemEnd,
                    PpiListBegin, PpiListEnd,
                    PeiHeapBegin, PeiHeapEnd,
                    PeiStackBegin,PeiStackEnd,
                    GlobalVariablePtrBegin, GlobalVariablePtrEnd,
                    SecStackBegin,SecStackEnd,
                    MonModeStackBegin, MonModeStackEnd);
}


VOID
EFIAPI
InitializeHOBs (
  IN CONST EFI_SEC_PEI_HAND_OFF    *SecCoreData,
  IN CONST EFI_PEI_PPI_DESCRIPTOR  *PpiList,
  IN CONST UINT64                   StartTimeStamp
  )
{
  EFI_STATUS                    Status;
  EFI_HOB_HANDOFF_INFO_TABLE*   HobList;
  ARM_MP_CORE_INFO_PPI*         ArmMpCoreInfoPpi;
  UINTN                         ArmCoreCount;
  ARM_CORE_INFO*                ArmCoreInfoTable;

  // Construct a Hob header at SecCoreData->PeiTemporaryRamBase
  HobList = HobConstructor (
    (VOID*)SecCoreData->PeiTemporaryRamBase,  // The lowest address location of temp memory
    SecCoreData->PeiTemporaryRamSize,         // The size of temp memory
    (VOID*)SecCoreData->PeiTemporaryRamBase,  // The lowest address for use by the HOB producer phase
    (VOID*)SecCoreData->StackBase             // The highest address is StackBase for use by the HOB producer phase
    );

  // Save the head of HobList so that we can retrieve later using PrePeiGetHobList
  PrePeiSetHobList (HobList);

  // Now, the HOB List has been initialized, we can register performance information
  PERF_START (NULL, "PEI", NULL, StartTimeStamp);

  // Put PEI stack base and size information into Hob
  BuildStackHob ((UINTN)SecCoreData->StackBase, SecCoreData->StackSize);

  // Put PeiGlobalVariable pointer base and size information into Hob
  BuildGlobalVariableHob (PcdGet32 (PcdCPUCoresStackBase) +
                          PcdGet32 (PcdCPUCorePrimaryStackSize) -
                          PcdGet32 (PcdPeiGlobalVariableSize),
                          PcdGet32 (PcdPeiGlobalVariableSize));

  // Put the CPU memory and io spaces sizes into Hob
  BuildCpuHob (PcdGet8 (PcdPrePiCpuMemorySize), PcdGet8 (PcdPrePiCpuIoSize));

  // Put the CPU MP Info table into Hob
  if (ArmIsMpCore ()) {
    // Only MP Core platform need to produce gArmMpCoreInfoPpiGuid
    Status = GetPlatformPpi (SecCoreData, PpiList, &gArmMpCoreInfoPpiGuid, (VOID**)&ArmMpCoreInfoPpi);

    // On MP Core Platform we must implement the ARM MP Core Info PPI (gArmMpCoreInfoPpiGuid)
    ASSERT_EFI_ERROR (Status);

    // Build the MP Core Info Table
    ArmCoreCount = 0;
    Status = ArmMpCoreInfoPpi->GetMpCoreInfo (&ArmCoreCount, &ArmCoreInfoTable);
    if (!EFI_ERROR(Status) && (ArmCoreCount > 0)) {
      // Build MPCore Info HOB
      BuildGuidDataHob (&gArmMpCoreInfoGuid, ArmCoreInfoTable, sizeof (ARM_CORE_INFO) * ArmCoreCount);
    }
  }

  // Put the EFI_BOOT_MODE information into hob
  SetBootMode (PlatformPeiGetBootMode ());

  // Build HOBs to pass up our version of stuff the DXE Core needs to save space
  BuildPeCoffLoaderHob ();
  BuildExtractSectionHob (
    &gLzmaCustomDecompressGuid,
    LzmaGuidedSectionGetInfo,
    LzmaGuidedSectionExtraction
    );

}


EFI_STATUS
EFIAPI
GetPlatformPpi (
IN CONST EFI_SEC_PEI_HAND_OFF      *SecCoreData,
  IN CONST EFI_PEI_PPI_DESCRIPTOR  *PpiList,
  IN  EFI_GUID                     *PpiGuid,
  OUT VOID                         **Ppi
  )
{
  UINTN                   PpiListSize;
  UINTN                   PpiListCount;
  UINTN                   Index;

  PpiListSize = (UINT32)SecCoreData->TemporaryRamBase -  PcdGet32 (PcdCPUCoresStackBase);
  PpiListCount = PpiListSize / sizeof(EFI_PEI_PPI_DESCRIPTOR);
  for (Index = 0; Index < PpiListCount; Index++, PpiList++) {
    if (CompareGuid (PpiList->Guid, PpiGuid) == TRUE) {
      *Ppi = PpiList->Ppi;
      return EFI_SUCCESS;
    }
  }

  return EFI_NOT_FOUND;
}

VOID
EFIAPI
BuildGlobalVariableHob (
  IN EFI_PHYSICAL_ADDRESS         GlobalVariableBase,
  IN UINT32                       GlobalVariableSize
  )
{
  ARM_HOB_GLOBAL_VARIABLE  *Hob;

  Hob = CreateHob (EFI_HOB_TYPE_GUID_EXTENSION, sizeof (ARM_HOB_GLOBAL_VARIABLE));
  ASSERT(Hob != NULL);

  CopyGuid (&(Hob->Header.Name), &gArmGlobalVariableGuid);
  Hob->GlobalVariableBase = GlobalVariableBase;
  Hob->GlobalVariableSize = GlobalVariableSize;
}


EFI_STATUS
EFIAPI
BuildSystemMemoryHOBs (
  IN EFI_PHYSICAL_ADDRESS     DramMemoryBase,
  IN UINT64                   DramMemorySize
  )
{
  EFI_RESOURCE_ATTRIBUTE_TYPE ResourceAttributes;

  // Declared the DRAM base, size and attributes into HOB
  ResourceAttributes = (
      EFI_RESOURCE_ATTRIBUTE_PRESENT |
      EFI_RESOURCE_ATTRIBUTE_INITIALIZED |
      EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE |
      EFI_RESOURCE_ATTRIBUTE_WRITE_COMBINEABLE |
      EFI_RESOURCE_ATTRIBUTE_WRITE_THROUGH_CACHEABLE |
      EFI_RESOURCE_ATTRIBUTE_WRITE_BACK_CACHEABLE |
      EFI_RESOURCE_ATTRIBUTE_TESTED
  );
  BuildResourceDescriptorHob (
      EFI_RESOURCE_SYSTEM_MEMORY,
      ResourceAttributes,
      DramMemoryBase,
      DramMemorySize
  );
  // Now, the permanent memory has been installed, we can call AllocatePages()

  // That's it, we are done building system memory Hobs
  // We do not want to enable MMU in boot loader phase or for DXE phase
  // If you want to enable MMU in UEFI phase, you will have to build a
  // Virtual Memory Map to initialize the MMU on your platform.
  // You can use the code in ArmPlatformPkg by MemoryInitPei Module
  // as a reference starting point and insider it here

  return EFI_SUCCESS;
}

VOID
EFIAPI
BootUnCompressedDxeFv (
  VOID
  )
{
  UINT32                DxeFileSize;
  UINTN                 DxeFvBase;
  EFI_FV_FILE_INFO      DxeCoreFileInfo;
  EFI_PHYSICAL_ADDRESS  DxeCoreAddress;
  UINT32                PECoffBase;
  EFI_PHYSICAL_ADDRESS  DxeCoreEntryPoint;

  // Load DXE FV to DRAM
  DxeFvBase = PcdGet32(PcdFdBaseAddress); // Defined in .FDF file
  LoadDxeImageToRam (DxeFvBase, &DxeFileSize);
  // Assert if this is not a DXE FV file matching this PEI
  ASSERT_PLATFORM_INIT(DxeFileSize == PcdGet32(PcdDxeFvSize));

  // Put DXE FV base and size information into Hob
  BuildFvHob (DxeFvBase, DxeFileSize);

  // Find the DXE Entry point
  DxeCoreAddress = DxeFvBase + 0x64; // FVH header size (0x48) + FFS header size (0x18) + Section header (0x04) = 0x64
  PECoffBase     = DxeFvBase + 0x64; // FVH header size (0x48) + FFS header size (0x18) + Section header (0x04) = 0x64
  CopyGuid (&DxeCoreFileInfo.FileName, (EFI_GUID*)(DxeFvBase + 0x48)); // EFI_GUID offset
  if ((*((UINT32*)(PECoffBase)) == 0x00005A4D) && (*((UINT32*)(PECoffBase + 0x80)) == 0x00004550))
  {
    // Found "MZ" and "PE" signature, prepare to boot DXE phase
    DxeCoreEntryPoint = (*((UINT32*)(PECoffBase + 0x80 + 0x34)) + *((UINT32*)(PECoffBase + 0x80 + 0x28)));
    SerialPortPrint ("DXE Core entry at 0x%08Lx\n", DxeCoreEntryPoint);

    // Add HOB for the DXE Core
    //
    BuildModuleHob (
      &DxeCoreFileInfo.FileName,
      DxeCoreAddress,
      ALIGN_VALUE (DxeFileSize, EFI_PAGE_SIZE),
      DxeCoreEntryPoint
      );

    // Load the DXE Core and transfer control to it
    ((DXE_CORE_ENTRY_POINT)(UINTN)DxeCoreEntryPoint) (PrePeiGetHobList());

  }

  // Should not reach here
  ASSERT_PLATFORM_INIT(0);
  // Dead loop
  EFI_DEADLOOP();
}


VOID
EFIAPI
BootCompressedDxeFv (
  VOID
  )
{
  EFI_STATUS                  Status;
  UINT32                      DxeFileSize;
  UINTN                       DxeDecompressedFvBase;
  UINTN                       DxeCompressedFvBase;
  EFI_FV_FILE_INFO            DxeCoreFileInfo;
  EFI_PHYSICAL_ADDRESS        DxeCoreAddress;
  UINT32                      PECoffBase;
  EFI_PHYSICAL_ADDRESS        DxeCoreEntryPoint;
  EFI_COMMON_SECTION_HEADER*  Section;
  VOID*                       OutputBuffer;
  UINT32                      OutputBufferSize;
  VOID*                       ScratchBuffer;
  UINT32                      ScratchBufferSize;
  UINT16                      SectionAttribute;
  UINT32                      AuthenticationStatus;

  // Load Compress DXE FV to DRAM
  // PcdFdBaseAddress is defined in .FDF file
  DxeDecompressedFvBase = PcdGet32(PcdFdBaseAddress);
  // Compressed FV is load 16 MB after decompression destination offset
  DxeCompressedFvBase = DxeDecompressedFvBase + 0x1000000;
  // LZMA Scratch Buffer
  ScratchBuffer = (VOID*)(UINTN)(DxeCompressedFvBase + 0x1000000);
  // LZMA Output Buffer
  OutputBuffer = (VOID*)(UINTN)(DxeDecompressedFvBase - sizeof(EFI_COMMON_SECTION_HEADER2));
  // Load the Compressed FV
  LoadDxeImageToRam (DxeCompressedFvBase, &DxeFileSize);
  // Assert if this is not a DXE FV file
  ASSERT_PLATFORM_INIT(DxeFileSize == PcdGet32(PcdDxeFvSize));
  // Verify is a compressed DXE FV
  Section = (EFI_COMMON_SECTION_HEADER*)(DxeCompressedFvBase + 0x60); // FVH header size (0x48) + FFS header size (0x18)
  Status = LzmaGuidedSectionGetInfo (Section, &OutputBufferSize, &ScratchBufferSize, &SectionAttribute);
  if (EFI_ERROR(Status)) {
    // Not a LZMA compressed DXE FV
    return;
  }
  // Decompress the FV
  LzmaGuidedSectionExtraction (Section, &OutputBuffer, ScratchBuffer, &AuthenticationStatus);

  // Put decompressed DXE FV base and size information into Hob
  BuildFvHob (DxeDecompressedFvBase, OutputBufferSize);

  // Find the DXE Entry point
  DxeCoreAddress = DxeDecompressedFvBase + 0x64; // FVH header size (0x48) + FFS header size (0x18) + Section header (0x04) = 0x64
  PECoffBase     = DxeDecompressedFvBase + 0x64; // FVH header size (0x48) + FFS header size (0x18) + Section header (0x04) = 0x64
  CopyGuid (&DxeCoreFileInfo.FileName, (EFI_GUID*)(DxeDecompressedFvBase + 0x48)); // EFI_GUID offset
  if ((*((UINT32*)(PECoffBase)) == 0x00005A4D) && (*((UINT32*)(PECoffBase + 0x80)) == 0x00004550))
  {
    // Found "MZ" and "PE" signature, prepare to boot DXE phase
    DxeCoreEntryPoint = (*((UINT32*)(PECoffBase + 0x80 + 0x34)) + *((UINT32*)(PECoffBase + 0x80 + 0x28)));
    SerialPortPrint ("DXE Core entry at 0x%08Lx\n", DxeCoreEntryPoint);

    //
    // Add HOB for the DXE Core
    //
    BuildModuleHob (
      &DxeCoreFileInfo.FileName,
      DxeCoreAddress,
      ALIGN_VALUE (DxeFileSize, EFI_PAGE_SIZE),
      DxeCoreEntryPoint
      );

    // Load the DXE Core and transfer control to it
    ((DXE_CORE_ENTRY_POINT)(UINTN)DxeCoreEntryPoint) (PrePeiGetHobList());

  }

  // Should not reach here
  ASSERT_PLATFORM_INIT(0);
  // Dead loop
  EFI_DEADLOOP();
}