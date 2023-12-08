/*========================== begin_copyright_notice ============================

Copyright (C) 2023 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#pragma once

#include "Compiler/MetaDataUtilsWrapper.h"
#include "Compiler/CodeGenContextWrapper.hpp"

#include "common/LLVMWarningsPush.hpp"
#include <llvm/Pass.h>
#include "common/LLVMWarningsPop.hpp"

namespace IGC {
/// @brief  This inserts calls to stack overflow detection builtins.
///
class StackOverflowDetectionPass : public llvm::ModulePass {
public:
  /// @brief  Pass identification.
  static char ID;

  static constexpr const char* STACK_OVERFLOW_INIT_BUILTIN_NAME = "__stackoverflow_init";
  static constexpr const char* STACK_OVERFLOW_DETECTION_BUILTIN_NAME = "__stackoverflow_detection";

  StackOverflowDetectionPass();
  ~StackOverflowDetectionPass() {}

  virtual llvm::StringRef getPassName() const override {
    return "StackOverflowDetectionPass";
  }

  virtual bool runOnModule(llvm::Module &M) override;

  virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const override {
    AU.addRequired<CodeGenContextWrapper>();
    AU.addRequired<MetaDataUtilsWrapper>();
  }
};

} // namespace IGC
