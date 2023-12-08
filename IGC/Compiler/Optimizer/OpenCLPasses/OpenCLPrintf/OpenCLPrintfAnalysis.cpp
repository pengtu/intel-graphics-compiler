/*========================== begin_copyright_notice ============================

Copyright (C) 2017-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include "AdaptorCommon/ImplicitArgs.hpp"
#include "Compiler/Optimizer/OpenCLPasses/OpenCLPrintf/OpenCLPrintfAnalysis.hpp"
#include "Compiler/IGCPassSupport.h"

#include "common/LLVMWarningsPush.hpp"
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Demangle/Demangle.h>
#include "common/LLVMWarningsPop.hpp"
#include <set>

using namespace llvm;
using namespace IGC;
using namespace IGC::IGCMD;

// Register pass to igc-opt
#define PASS_FLAG "igc-opencl-printf-analysis"
#define PASS_DESCRIPTION "Analyzes OpenCL printf calls"
#define PASS_CFG_ONLY false
#define PASS_ANALYSIS false
IGC_INITIALIZE_PASS_BEGIN(OpenCLPrintfAnalysis, PASS_FLAG, PASS_DESCRIPTION, PASS_CFG_ONLY, PASS_ANALYSIS)
IGC_INITIALIZE_PASS_DEPENDENCY(MetaDataUtilsWrapper)
IGC_INITIALIZE_PASS_END(OpenCLPrintfAnalysis, PASS_FLAG, PASS_DESCRIPTION, PASS_CFG_ONLY, PASS_ANALYSIS)

char OpenCLPrintfAnalysis::ID = 0;

OpenCLPrintfAnalysis::OpenCLPrintfAnalysis() : ModulePass(ID)
{
    initializeOpenCLPrintfAnalysisPass(*PassRegistry::getPassRegistry());
}

//TODO: move to a common place
const StringRef OpenCLPrintfAnalysis::OPENCL_PRINTF_FUNCTION_NAME = "printf";
const StringRef OpenCLPrintfAnalysis::ONEAPI_PRINTF_FUNCTION_NAME =
    "ext::oneapi::experimental::printf";
const StringRef OpenCLPrintfAnalysis::BUILTIN_PRINTF_FUNCTION_NAME =
"__builtin_IB_printf_to_buffer";

bool OpenCLPrintfAnalysis::isOpenCLPrintf(const llvm::Function *F)
{
    return F->getName() == OPENCL_PRINTF_FUNCTION_NAME;
}

bool OpenCLPrintfAnalysis::isOneAPIPrintf(const llvm::Function *F)
{
    std::string demangledName = llvm::demangle(F->getName().str());
    return demangledName.find(ONEAPI_PRINTF_FUNCTION_NAME.data()) != std::string::npos;
}

bool OpenCLPrintfAnalysis::isBuiltinPrintf(const llvm::Function* F)
{
    return F->getName() == BUILTIN_PRINTF_FUNCTION_NAME;
}

bool OpenCLPrintfAnalysis::runOnModule(Module& M)
{
    m_pMDUtils = getAnalysis<MetaDataUtilsWrapper>().getMetaDataUtils();

    visit(M);
    bool changed = false;
    if (m_hasPrintfs.size())
    {
        for (Function& func : M.getFunctionList())
        {
            if (!func.isDeclaration() &&
                m_hasPrintfs.find(&func) != m_hasPrintfs.end())
            {
                addPrintfBufferArgs(func);
                changed = true;
            }
        }
    }

    // Update LLVM metadata based on IGC MetadataUtils
    if (changed)
        m_pMDUtils->save(M.getContext());

    return m_hasPrintfs.size();
}

void OpenCLPrintfAnalysis::visitCallInst(llvm::CallInst& callInst)
{
    Function* pF = callInst.getParent()->getParent();
    if (!callInst.getCalledFunction() || m_hasPrintfs.find(pF)!=m_hasPrintfs.end())
    {
        return;
    }

    StringRef  funcName = callInst.getCalledFunction()->getName();
    bool hasPrintf = (funcName == OpenCLPrintfAnalysis::OPENCL_PRINTF_FUNCTION_NAME);
    if (hasPrintf)
    {
        m_hasPrintfs.insert(pF);
    }
}

void OpenCLPrintfAnalysis::addPrintfBufferArgs(Function& F)
{
    SmallVector<ImplicitArg::ArgType, 1> implicitArgs;
    implicitArgs.push_back(ImplicitArg::PRINTF_BUFFER);
    ImplicitArgs::addImplicitArgs(F, implicitArgs, m_pMDUtils);
}

bool isPrintfOnlyStringConstantImpl(const llvm::Value *v, std::set<const llvm::User *>& visited)
{
    // Recursively check the users of the value until reaching the top level
    // user or a call.

    // Base case: Return false when use list is empty.
    if (v->use_empty())
    {
        return false;
    }

    // Check users recursively with a list of permitted in-between uses. Here we
    // follow OpenCLPrintfResolution::argIsString() to check if they are one of
    // CastInst, GEP with all-zero indices, SelectInst, and PHINode.
    for (auto& use : v->uses())
    {
        auto user = use.getUser();
        // Skip if the user is visited.
        if (visited.count(user))
            continue;
        visited.insert(user);

        bool res = false;
        if (const llvm::CallInst *call = llvm::dyn_cast<llvm::CallInst>(user))
        {
            // Stop when reaching a call and check if it is an opencl/oneapi
            // printf call.
            const Function* target = call->getCalledFunction();
            bool isStringLiteral = OpenCLPrintfAnalysis::isOpenCLPrintf(target) ||
                OpenCLPrintfAnalysis::isOneAPIPrintf(target) ||
                OpenCLPrintfAnalysis::isBuiltinPrintf(target);

            if (isStringLiteral)
            {
                res = true;
            }
            else
            {
                unsigned int opIndex = call->getDataOperandNo(&use);
                res = isPrintfOnlyStringConstantImpl(target->arg_begin() + opIndex, visited);
            }
        }
        else if (llvm::dyn_cast<llvm::CastInst>(user) ||
                 llvm::dyn_cast<llvm::SelectInst>(user) ||
                 llvm::dyn_cast<llvm::PHINode>(user))
        {
            res = isPrintfOnlyStringConstantImpl(user, visited);
        }
        else if (const llvm::GetElementPtrInst *gep = llvm::dyn_cast<llvm::GetElementPtrInst>(user))
        {
            if (gep->hasAllZeroIndices())
                res = isPrintfOnlyStringConstantImpl(user, visited);
        }

        if (!res)
            return false;
    }

    // Return true as every top level user is a printf call.
    return true;
}

// Check paths from a string literal to printf calls and return true if every
// path lead to a printf call.
bool OpenCLPrintfAnalysis::isPrintfOnlyStringConstant(const llvm::GlobalVariable *GV)
{
    const llvm::Constant *initializer = GV->getInitializer();
    if (!initializer)
        return false;
    const llvm::ConstantDataSequential* cds = llvm::dyn_cast<llvm::ConstantDataSequential>(initializer);
    if (!cds || !cds->isCString() || !cds->isString())
        return false;

    std::set<const llvm::User *> visited;
    return isPrintfOnlyStringConstantImpl(GV, visited);
}
