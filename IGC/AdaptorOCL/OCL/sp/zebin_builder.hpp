/*========================== begin_copyright_notice ============================

Copyright (C) 2020-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#pragma once

#include <vector>
#include <ZEELFObjectBuilder.hpp>
#include "sp_g8.h"
#include "llvm/BinaryFormat/ELF.h"
#include "CLElfLib/ElfReader.h"

namespace IGC
{
    struct SOpenCLKernelInfo;
    struct SOpenCLProgramInfo;
    class CBTILayout;
    class OpenCLProgramContext;
}

namespace vISA
{
    struct ZESymEntry;
    struct ZEFuncAttribEntry;
}

namespace iOpenCL
{

/// ZEBinaryBuilder - Provides services to create a ZE Binary from given
/// SProgramOutput information
class ZEBinaryBuilder : DisallowCopy
{
public:
    // Setup ZEBin platform, and ELF header information. The program scope information
    // is also be parsed from SOpenCLProgramInfo in the constructor
    ZEBinaryBuilder(const PLATFORM plat, bool is64BitPointer,
        const IGC::SOpenCLProgramInfo& programInfo,
        const uint8_t* spvData,      uint32_t spvSize,
        const uint8_t* metricsData,  uint32_t metricsSize,
        const uint8_t* buildOptions, uint32_t buildOptionsSize);

    // Set the ProductFamily as the specified value.
    void setProductFamily(PRODUCT_FAMILY value);

    // Set the GfxCoreFamily as the specified value.
    void setGfxCoreFamily(GFXCORE_FAMILY value);

    // Set VISA ABI version used in generated code.
    void setVISAABIVersion(unsigned int ver);

    // Set the GmdId as the specified value.
    void setGmdID(GFX_GMD_ID value);

    // Pair of name for the section (1st elem) and VISA asm text (2nd elem).
    using NamedVISAAsm = std::pair<std::string, std::string>;

    /// add kernel information. Also create kernel metadata information for .ze_info
    /// This function can be called several times for adding different kernel information
    /// into this ZEObject
    /// The given rawIsaBinary must be lived through the entire ZEBinaryBuilder life
    void createKernel(
        const char*  rawIsaBinary,
        unsigned int rawIsaBinarySize,
        const IGC::SOpenCLKernelInfo& annotations,
        const uint32_t grfSize,
        const IGC::CBTILayout& layout,
        const std::vector<NamedVISAAsm>& visaasm,
        bool isProgramDebuggable);

    // getElfSymbol - find a symbol name in ELF binary and return a symbol entry
    // that will later be transformed to ZE binary format
    void getElfSymbol(CLElfLib::CElfReader* elfReader, const unsigned int symtabIdx, llvm::ELF::Elf64_Sym& symtabEntry,
        char*& symName);

    /// addElfSections - copy every section of ELF file (a buffer in memory) to zeBinary
    void addElfSections(void* elfBin, size_t elfSize);

    /// getBinaryObject - get the final ze object
    void getBinaryObject(llvm::raw_pwrite_stream& os);

    // getBinaryObject - write the final object into given Util::BinaryStream
    // Avoid using this function, which has extra buffer copy
    void getBinaryObject(Util::BinaryStream& outputStream);

    void printBinaryObject(const std::string& filename);

    // print .ze_info to given os
    void printZEInfo(llvm::raw_ostream &os);

    // print .ze_info into a file with given filename
    void printZEInfo(const std::string &filename);

private:
    /// ------------ program scope helper functions ------------
    /// add program scope information. This function will be called in the ctor.
    /// The program scope information include global buffers (data sections for
    /// globals and global constants)
    /// ProgramScopeInfo must be prepared before kernel information. For example,
    /// Symbols are per-kernel information but they could possible refering to
    /// program-scope sections such as global buffer.
    void addProgramScopeInfo(const IGC::SOpenCLProgramInfo& programInfo);

    /// add data section for global constants
    void addGlobalConstants(const IGC::SOpenCLProgramInfo& annotations);

    /// add data section for globals
    void addGlobals(const IGC::SOpenCLProgramInfo& annotations);

    /// add spir-v section
    void addSPIRV(const uint8_t* data, uint32_t size);

    /// add miscellaneous info section (section with SHT_ZEBIN_MISC type)
    void addMiscInfoSection(std::string sectName, const uint8_t* data, uint32_t size);

    /// add runtime symbols
    void addRuntimeSymbols(const IGC::SOpenCLProgramInfo& annotations);

    /// add note section for IGC metrics
    void addMetrics(const uint8_t* data, uint32_t size);

    /// add program scope symbols (e.g. symbols defined in global/const buffer)
    void addProgramSymbols(const IGC::SOpenCLProgramInfo& annotations);

    /// add program scope relocations (e.g. relocations for global/const buffer)
    void addProgramRelocations(const IGC::SOpenCLProgramInfo& annotations);

    /// ------------ kernel scope helper functions ------------
    /// add gen binary
    zebin::ZEELFObjectBuilder::SectionID addKernelBinary(
        const std::string& kernelName, const char* kernelBinary,
        unsigned int kernelBinarySize);

    /// add user attributes (kernel attributes)
    void addUserAttributes(const IGC::SOpenCLKernelInfo& annotations,
                           zebin::zeInfoKernel& zeinfoKernel);

    /// add kernel execution environment
    void addKernelExecEnv(const IGC::SOpenCLKernelInfo& annotations,
                          zebin::zeInfoKernel& zeinfoKernel);

    /// add execution environment for external function
    void addFunctionExecEnv(const IGC::SOpenCLKernelInfo& annotations,
                            const vISA::ZEFuncAttribEntry& zeFuncAttr,
                            zebin::zeInfoFunction& zeFunction);

    /// add experimental properties
    void addKernelExperimentalProperties(
        const IGC::SOpenCLKernelInfo& annotations,
        zebin::zeInfoKernel& zeinfoKernel);

    /// add symbols of this kernel corresponding to kernel binary
    /// added by addKernelBinary
    void addKernelSymbols(
        zebin::ZEELFObjectBuilder::SectionID kernelSectId,
        const IGC::SOpenCLKernelInfo& annotations);

    /// get symbol type
    /// FIXME: this should be decided when symbol being created
    uint8_t getSymbolElfType(const vISA::ZESymEntry& sym);

    /// addSymbol - a helper function to add a symbol which is defined in targetSect
    void addSymbol(const vISA::ZESymEntry& sym, uint8_t binding,
        zebin::ZEELFObjectBuilder::SectionID targetSect);

    /// add relocations of this kernel corresponding to binary added by
    /// addKernelBinary.
    void addKernelRelocations(
        zebin::ZEELFObjectBuilder::SectionID targetId,
        const IGC::SOpenCLKernelInfo& annotations);

    /// add local ids as per-thread payload argument
    void addLocalIds(uint32_t simdSize, uint32_t grfSize,
        bool has_local_id_x, bool has_local_id_y, bool has_local_id_z,
        zebin::zeInfoKernel& zeinfoKernel);

    /// add payload arguments and BTI info from IGC::SOpenCLKernelInfo
    /// payload arguments and BTI info have been added at
    /// COpenCLKernel::CreateZEPayloadArguments
    void addPayloadArgsAndBTI(
        const IGC::SOpenCLKernelInfo& annotations,
        zebin::zeInfoKernel& zeinfoKernel);

    /// add inline samplers in IGC::SOpenCLKernelInfo created from
    /// COpenCLKernel::CreateZEInlineSamplerPayloadArguments()
    void addInlineSamplers(
        const IGC::SOpenCLKernelInfo& annotations,
        zebin::zeInfoKernel& zeinfoKernel);

    /// add Memory buffer information
    void addMemoryBuffer(
        const IGC::SOpenCLKernelInfo& annotations,
        zebin::zeInfoKernel& zeinfoKernel);

    /// add gtpin_info section
    /// Add everything used to be in patch token iOpenCL::PATCH_TOKEN_GTPIN_INFO
    /// into gtpin_info section
    void addGTPinInfo(const IGC::SOpenCLKernelInfo& annotations);

    /// Add function attributes for external functions.
    void addFunctionAttrs(const IGC::SOpenCLKernelInfo& annotations);

    /// check if the kernel has misc info. The entry of this function in
    /// kernels_misc_info should only be created when this function return
    /// true
    bool hasKernelMiscInfo(const IGC::SOpenCLKernelInfo &annotations) const;

    /// Add kernel arg info
    void addKernelArgInfo(
        const IGC::SOpenCLKernelInfo& annotations,
        zebin::zeInfoKernelMiscInfo& zeinfoKernelMisc);

    /// Calculate correct (pure) size of ELF binary, because m_debugDataSize in kernel output
    /// contains something else.
    size_t calcElfSize(void* elfBin, size_t elfSize);

    /// add debug environment
    void addKernelDebugEnv(const IGC::SOpenCLKernelInfo& annotations,
                           const IGC::CBTILayout& layout,
                           zebin::zeInfoKernel& zeinfoKernel);

    /// add visasm of the kernel
    void addKernelVISAAsm(const std::string& kernel, const std::string& visaasm);

    /// add global_host_access_table section to .ze_info
    void addGlobalHostAccessInfo(const IGC::SOpenCLProgramInfo& annotations);

private:
    // mBuilder - Builder of a ZE ELF object
    zebin::ZEELFObjectBuilder mBuilder;

    // mZEInfoBuilder - Builder and holder of a zeInfoContainer, which will
    // be added into ZEELFObjectBuilder as .ze_info section
    zebin::ZEInfoBuilder mZEInfoBuilder;

    const PLATFORM mPlatform;
    G6HWC::SMediaHardwareCapabilities mHWCaps;

    /// sectionID holder for program scope sections
    /// There should be only one global, global constant buffer per program
    zebin::ZEELFObjectBuilder::SectionID mGlobalConstSectID = -1;
    zebin::ZEELFObjectBuilder::SectionID mConstStringSectID = -1;
    zebin::ZEELFObjectBuilder::SectionID mGlobalSectID = -1;
};

// a helper function to get ZE image type from a OCL image type
zebin::PreDefinedAttrGetter::ArgImageType getZEImageType(iOpenCL::IMAGE_MEMORY_OBJECT_TYPE);

// a helper function to get ZE sampler type from a OCL sampler type
zebin::PreDefinedAttrGetter::ArgSamplerType getZESamplerType(iOpenCL::SAMPLER_OBJECT_TYPE);

} //namespace iOpenCL
