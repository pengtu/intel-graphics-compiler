/*========================== begin_copyright_notice ============================

Copyright (C) 2017-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

/*========================== begin_copyright_notice ============================

This file is distributed under the University of Illinois Open Source License.
See LICENSE.TXT for details.

============================= end_copyright_notice ===========================*/

/*========================== begin_copyright_notice ============================

Copyright (C) 2014 Advanced Micro Devices, Inc. All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal with the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimers.
Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimers in the documentation
and/or other materials provided with the distribution.
Neither the names of Advanced Micro Devices, Inc., nor the names of its
contributors may be used to endorse or promote products derived from this
Software without specific prior written permission.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS WITH
THE SOFTWARE.

============================= end_copyright_notice ===========================*/

// This file implements the types defined in SPIRV spec with op codes.

#include "SPIRVType.h"
#include "SPIRVDecorate.h"
#include "SPIRVValue.h"
#include "Probe/Assertion.h"

namespace igc_spv{

SPIRVType*
SPIRVType::getArrayElementType() const {
  IGC_ASSERT_MESSAGE(OpCode == OpTypeArray, "Not array type");
  return static_cast<const SPIRVTypeArray *const>(this)->getElementType();
}

uint64_t
SPIRVType::getArrayLength() const {
  IGC_ASSERT_MESSAGE(OpCode == OpTypeArray, "Not array type");
  const SPIRVTypeArray* AsArray = static_cast<const SPIRVTypeArray*>(this);
  IGC_ASSERT_MESSAGE(AsArray->getLength()->getOpCode() == OpConstant,
      "getArrayLength can only be called with constant array lengths");
  return AsArray->getLength()->getZExtIntValue();
}

SPIRVWord
SPIRVType::getBitWidth() const {
  if (isTypeVector())
    return getVectorComponentType()->getBitWidth();
  if (isTypeBool())
    return 1;
  return isTypeInt()? getIntegerBitWidth() : getFloatBitWidth();
}

SPIRVWord
SPIRVType::getFloatBitWidth()const {
  IGC_ASSERT_MESSAGE(OpCode == OpTypeFloat, "Not an integer type");
  return static_cast<const SPIRVTypeFloat *const>(this)->getBitWidth();
}

SPIRVWord
SPIRVType::getIntegerBitWidth()const {
  IGC_ASSERT_MESSAGE((OpCode == OpTypeInt || OpCode == OpTypeBool), "Not an integer type");
  if (isTypeBool())
    return 1;
  return static_cast<const SPIRVTypeInt *const>(this)->getBitWidth();
}

SPIRVType *
SPIRVType::getFunctionReturnType() const {
  IGC_ASSERT(OpCode == OpTypeFunction);
  return static_cast<const SPIRVTypeFunction *const>(this)->getReturnType();
}

SPIRVType *
SPIRVType::getPointerElementType()const {
  IGC_ASSERT_MESSAGE(OpCode == OpTypePointer, "Not a pointer type");
  return static_cast<const SPIRVTypePointer *const>(this)->getElementType();
}

SPIRVStorageClassKind
SPIRVType::getPointerStorageClass() const {
  IGC_ASSERT_MESSAGE(OpCode == OpTypePointer, "Not a pointer type");
  return static_cast<const SPIRVTypePointer *const>(this)->getStorageClass();
}

SPIRVType*
SPIRVType::getStructMemberType(size_t Index) const {
  IGC_ASSERT_MESSAGE(OpCode == OpTypeStruct, "Not struct type");
  return static_cast<const SPIRVTypeStruct *const>(this)->getMemberType(Index);
}

SPIRVWord
SPIRVType::getStructMemberCount() const {
  IGC_ASSERT_MESSAGE(OpCode == OpTypeStruct, "Not struct type");
  return static_cast<const SPIRVTypeStruct *const>(this)->getMemberCount();
}

SPIRVWord
SPIRVType::getVectorComponentCount() const {
  IGC_ASSERT_MESSAGE(OpCode == OpTypeVector, "Not vector type");
  return static_cast<const SPIRVTypeVector *const>(this)->getComponentCount();
}

SPIRVType*
SPIRVType::getVectorComponentType() const {
  IGC_ASSERT_MESSAGE(OpCode == OpTypeVector, "Not vector type");
  return static_cast<const SPIRVTypeVector *const>(this)->getComponentType();
}

bool
SPIRVType::isTypeVoid() const {
  return OpCode == OpTypeVoid;
}
bool
SPIRVType::isTypeArray() const {
  return OpCode == OpTypeArray;
}

bool
SPIRVType::isTypeBool()const {
  return OpCode == OpTypeBool;
}

bool
SPIRVType::isTypeComposite() const {
  return isTypeVector() || isTypeArray() || isTypeStruct();
}

bool
SPIRVType::isTypeFloat(unsigned Bits)const {
  return isType<SPIRVTypeFloat>(this, Bits);
}

bool
SPIRVType::isTypeOCLImage()const {
  return isTypeImage() && static_cast<const SPIRVTypeImage *>(this)->
    isOCLImage();
}

bool
SPIRVType::isTypePipe()const {
  return OpCode == OpTypePipe;
}

bool
SPIRVType::isTypeReserveId() const {
  return OpCode == OpTypeReserveId;
}

bool
SPIRVType::isTypeInt(unsigned Bits)const {
  return isType<SPIRVTypeInt>(this, Bits);
}

bool
SPIRVType::isTypePointer()const {
  return OpCode == OpTypePointer;
}

bool
SPIRVType::isTypeOpaque()const {
  return OpCode == OpTypeOpaque;
}

bool
SPIRVType::isTypeEvent()const {
  return OpCode == OpTypeEvent;
}

bool
SPIRVType::isTypeDeviceEvent()const {
  return OpCode == OpTypeDeviceEvent;
}

bool
SPIRVType::isTypeSampler()const {
  return OpCode == OpTypeSampler;
}

bool
SPIRVType::isTypeImage()const {
  return OpCode == OpTypeImage;
}

bool
SPIRVType::isTypeSampledImage() const {
  return OpCode == OpTypeSampledImage;
}

bool
SPIRVType::isTypeVmeImageINTEL() const {
  return OpCode == OpTypeVmeImageINTEL;
}

bool
SPIRVType::isTypeSubgroupAvcINTEL() const {
    return isSubgroupAvcINTELTypeOpCode(OpCode);
}

bool
SPIRVType::isTypeStruct() const {
  return OpCode == OpTypeStruct;
}

bool
SPIRVType::isTypeVector() const {
  return OpCode == OpTypeVector;
}

bool
SPIRVType::isTypeNamedBarrier() const {
  return OpCode == OpTypeNamedBarrier;
}

bool
SPIRVType::isTypeQueue() const {
  return OpCode == OpTypeQueue;
}

bool
SPIRVType::isTypeVectorBool() const {
  return isTypeVector() && getVectorComponentType()->isTypeBool();
}

bool
SPIRVType::isTypeVectorInt() const {
  return isTypeVector() && getVectorComponentType()->isTypeInt();
}

bool
SPIRVType::isTypeVectorFloat() const {
  return isTypeVector() && getVectorComponentType()->isTypeFloat();
}

bool
SPIRVType::isTypeVectorOrScalarBool() const {
  return isTypeBool() || isTypeVectorBool();
}

bool
SPIRVType::isTypeVectorOrScalarInt() const {
  return isTypeInt() || isTypeVectorInt();
}

bool
SPIRVType::isTypeVectorOrScalarFloat() const {
  return isTypeFloat() || isTypeVectorFloat();
}

void SPIRVTypeStruct::decode(std::istream &I)
{
    SPIRVDecoder Decoder = getDecoder(I);
    Decoder >> Id;

    for (size_t i = 0, e = MemberTypeVec.size(); i != e; ++i)
    {
        SPIRVId currId;
        Decoder >> currId;

        if (Decoder.M.exist(currId))
        {
            SPIRVEntry* Entry = Decoder.M.getEntry(currId);
            MemberTypeVec[i] = static_cast<SPIRVType*>(Entry);
        }
        else
        {
            MemberTypeVec[i] = nullptr;
            Decoder.M.addUnknownStructField(this, i, currId);
        }
    }

    Module->add(this);

    Decoder.getWordCountAndOpCode();
    while (!I.eof()) {

        SPIRVEntry* Entry = Decoder.getEntry();
        if (Entry != nullptr)
        {
            Module->add(Entry);
        }
        if (Entry && Decoder.OpCode == ContinuedOpCode) {
            auto ContinuedInst = static_cast<ContinuedInstType>(Entry);
            addContinuedInstruction(ContinuedInst);
            Decoder.getWordCountAndOpCode();
        }
        else {
            break;
        }
    }
}

bool
SPIRVTypeStruct::isPacked() const {
  return hasDecorate(DecorationCPacked);
}

void
SPIRVTypeStruct::setPacked(bool Packed) {
  if (Packed)
    addDecorate(new SPIRVDecorate(DecorationCPacked, this));
  else
    eraseDecorate(DecorationCPacked);
}

SPIRVTypeArray::SPIRVTypeArray(SPIRVModule *M, SPIRVId TheId, SPIRVType *TheElemType,
        SPIRVConstant* TheLength)
      :SPIRVType(M, 4, OpTypeArray, TheId), ElemType(TheElemType),
       Length(TheLength->getId()){
      validate();
    }

void
SPIRVTypeArray::validate()const {
  SPIRVEntry::validate();
  ElemType->validate();
  IGC_ASSERT(getValue(Length)->getType()->isTypeInt());
}

SPIRVConstant*
SPIRVTypeArray::getLength() const {
  return get<SPIRVConstant>(Length);
}

_SPIRV_IMP_DEC3(SPIRVTypeArray, Id, ElemType, Length)

void SPIRVTypeForwardPointer::decode(std::istream& I) {
  auto Decoder = getDecoder(I);
  Decoder >> PointerId >> SC;
}

unsigned SPIRVTypeJointMatrixINTEL::getRows() const {
  return (unsigned)get<SPIRVConstant>(Args[0])->getZExtIntValue();
}

unsigned SPIRVTypeJointMatrixINTEL::getColumns() const {
  return (unsigned)get<SPIRVConstant>(Args[1])->getZExtIntValue();
}

unsigned SPIRVTypeJointMatrixINTEL::getLayout() const {
  if (isLayoutParameterPresent())
    return (unsigned)get<SPIRVConstant>(Args[2])->getZExtIntValue();
  return 0;
}

unsigned SPIRVTypeJointMatrixINTEL::getScope() const {
  return getOpCode() == OpTypeJointMatrixINTEL
             ? (unsigned)get<SPIRVConstant>(Args[2])->getZExtIntValue()
             : (unsigned)get<SPIRVConstant>(Args[3])->getZExtIntValue();
}

unsigned SPIRVTypeJointMatrixINTEL::getUse() const {
  if (isUseParameterPresent())
    return getOpCode() == OpTypeJointMatrixINTEL
               ? (unsigned)get<SPIRVConstant>(Args[3])->getZExtIntValue()
               : (unsigned)get<SPIRVConstant>(Args[4])->getZExtIntValue();
  return 0;
}

unsigned SPIRVTypeJointMatrixINTEL::getComponentTypeInterpretation() const {
  if (isComponentTypeInterpretationParameterPresent())
    return (unsigned)get<SPIRVConstant>(Args[4])->getZExtIntValue();
  return 0;
}

bool SPIRVTypeJointMatrixINTEL::isLayoutParameterPresent() const {
  return getOpCode() == OpTypeJointMatrixINTEL_OLD;
}

bool SPIRVTypeJointMatrixINTEL::isUseParameterPresent() const {
  return getOpCode() == OpTypeJointMatrixINTEL || Args.size() > 4;
}

bool SPIRVTypeJointMatrixINTEL::isComponentTypeInterpretationParameterPresent() const {
  return getOpCode() == OpTypeJointMatrixINTEL && Args.size() > 4;
}

std::string SPIRVTypeJointMatrixINTEL::getMangledElemType() const
{
  std::string type;

  if (ElemType->isTypeFloat())
      type += "f";
  else
      type += "i";
  type += std::to_string(ElemType->getBitWidth());

  return std::move(type);
}

std::string SPIRVTypeJointMatrixINTEL::getMangledName() const {
    std::string name;
    if (isUseParameterPresent()) {
      switch (getUse()) {
        case SPIRVTypeJointMatrixINTEL::UseMatrixA:
          name += "packedA_";
          break;
        case SPIRVTypeJointMatrixINTEL::UseMatrixB:
          name += "packedB_";
          break;
        case SPIRVTypeJointMatrixINTEL::UseAccumulator:
          name += "acc_";
          break;
      }
    } else {
      switch (getLayout()) {
        case SPIRVTypeJointMatrixINTEL::LayoutPackedA:
          name += "packedA_";
          break;
        case SPIRVTypeJointMatrixINTEL::LayoutPackedB:
          name += "packedB_";
          break;
        case SPIRVTypeJointMatrixINTEL::LayoutRowMajor:
        case SPIRVTypeJointMatrixINTEL::LayoutColumnMajor:
          name += "acc_";
          break;
      }
    }
    name += std::to_string(getRows());
    name += "x";
    name += std::to_string(getColumns());
    name += "_";

    name += getMangledElemType();
    return std::move(name);
}
}

