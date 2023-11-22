/*========================== begin_copyright_notice ============================

Copyright (C) 2017-2022 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/
#include "GenIntrinsics.h"

#include "GenIntrinsicFunctions.h"
#include "GenIntrinsicLookup.h"
#include "Probe/Assertion.h"
#include "Compiler/CodeGenPublic.h"

#include "common/LLVMWarningsPush.hpp"
#include <llvm/IR/Function.h>
#include "common/LLVMWarningsPop.hpp"


namespace IGC
{

static llvm::GenISAIntrinsic::ID GetID(const llvm::Function& func)
{
    LLVMContextWrapper& ctxWrapper = static_cast<LLVMContextWrapper&>(func.getContext());
    LLVMContextWrapper::SafeIntrinsicIDCacheTy& SafeIntrinsicIdCache = ctxWrapper.m_SafeIntrinsicIDCache;

    //If you do not find the function ptr as key corresponding to the GenISAIntrinsic::ID add the new key
    auto it = SafeIntrinsicIdCache.find(&func);
    if (it == SafeIntrinsicIdCache.end()) {
        llvm::GenISAIntrinsic::ID id = llvm::GenISAIntrinsic::ID::no_intrinsic;
        const llvm::ValueName* const pValueName = func.getValueName();
        if (nullptr != pValueName)
        {
            llvm::StringRef prefix = GetIntrinsicPrefixName();
            llvm::StringRef Name = pValueName->getKey();
            if (Name.size() > prefix.size() && Name.startswith(prefix))
            {
                id = static_cast<llvm::GenISAIntrinsic::ID>(LookupIntrinsicId(Name.data()));
                SafeIntrinsicIdCache[&func] = static_cast<uint32_t>(id);
            }
        }
        return id;
    }
    else
    {
        // If you have an entry for the function ptr corresponding to the GenISAIntrinsic::ID return it back,
        //instead of going through a lengthy look-up.
        return (static_cast<llvm::GenISAIntrinsic::ID>(it->second));
    }
}

} // namespace IGC

namespace llvm
{
namespace GenISAIntrinsic
{

std::string getName(ID id, ArrayRef<Type*> OverloadedTys /*= None*/)
{
    return IGC::GetName(id, OverloadedTys);
}

IntrinsicComments getIntrinsicComments(ID id)
{
    return IGC::GetIntrinsicComments(id);
}

Function* getDeclaration(Module* M, ID id, ArrayRef<Type*> OverloadedTys /*= None*/)
{
    return IGC::GetDeclaration(M, id, OverloadedTys);
}

ID getIntrinsicID(const Function* F)
{
    if (F != nullptr)
    {
        return IGC::GetID(*F);
    }
    return ID::no_intrinsic;
}

} // namespace GenISAIntrinsic
} // namespace llvm
