/*========================== begin_copyright_notice ============================

Copyright (C) 2020-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef _IGA_BACKEND_MESSAGES_MESSAGEDECODER_HPP_
#define _IGA_BACKEND_MESSAGES_MESSAGEDECODER_HPP_

#include "../../Frontend/IRToString.hpp"
#include "../../IR/Messages.hpp"
#include "../../Models/Models.hpp"
#include "../../asserts.hpp"
#include "../Native/Field.hpp"

#include <algorithm>
#include <functional>
#include <sstream>
#include <tuple>
#include <vector>

namespace iga {
using DescFieldFormatter = std::function<void(std::stringstream &, uint32_t)>;
static inline void NO_DECODE(std::stringstream &, uint32_t) {}

struct MessageDecoder {
  // inputs
  const Model &decodeModel;
  const SFID sfid;
  ExecSize instExecSize;
  const SendDesc desc, exDesc;
  uint32_t exImmOffDesc;

  // outputs
  DecodeResult &result;

  const int DEFAULT_EXEC_SIZE, BITS_PER_REGISTER;

  MessageDecoder(Platform _platform, SFID _sfid, ExecSize _instExecSize,
                 uint32_t _exImmOffDesc, SendDesc _exDesc, SendDesc _desc,
                 DecodeResult &_result)
      : decodeModel(Model::LookupModelRef(_platform)), sfid(_sfid),
        instExecSize(_instExecSize), desc(_desc), exDesc(_exDesc),
        exImmOffDesc(_exImmOffDesc), result(_result),
        DEFAULT_EXEC_SIZE((_platform >= Platform::XE_HPC) ? 32 : 16),
        BITS_PER_REGISTER((_platform >= Platform::XE_HPC) ? 512 : 256) {
    result.info.op = SendOp::INVALID;
    result.info.cachingL3 = result.info.cachingL1 = CacheOpt::DEFAULT;
    result.info.elemSizeBitsRegFile = result.info.elemSizeBitsMemory = 0;
    result.info.channelsEnabled = result.info.elemsPerAddr = 0;
    result.info.execWidth =
        _instExecSize != ExecSize::INVALID ? int(_instExecSize) : 0;
    result.info.attributeSet = MessageInfo::Attr::NONE;
    result.info.addrType = AddrType::FLAT;
    result.info.surfaceId = 0;
    result.info.immediateOffset = 0;
    //
    // syntax.sfid = _sfid;
    result.syntax.controls = "." + ToSyntax(_sfid);
    //
    decodePayloadSizes();
  }

  Platform platform() const { return model().platform; }

  const Model &model() const { return decodeModel; }

  bool platformInRange(Platform lo, Platform hi) const {
    return platform() >= lo && platform() <= hi;
  }

  void setDoc(const char *doc) { setDoc(doc, doc, doc); }
  void setDoc(const char *preXe, const char *xe, const char *xe23) {
    addDoc(DocRef::INVALID, nullptr, chooseDoc(preXe, xe, xe23));
  }
  void addDocXe(DocRef::Kind k, const char *name, const char *linkXe,
                const char *linkXe2) {
    if (platform() >= Platform::XE2) {
      addDoc(k, name, linkXe2);
    } else {
      addDoc(k, name, linkXe);
    }
  }
  void addDoc(DocRef::Kind k, const char *name, const char *link) {
    if (k == DocRef::INVALID && !name && !link)
      return;
    result.info.refs.emplace_back(k, name, link);
  }
  void addDocs(DocRef::Kind k0, const char *name0, const char *link0,
               DocRef::Kind k1, const char *name1, const char *link1,
               DocRef::Kind k2 = DocRef::INVALID, const char *name2 = nullptr,
               const char *link2 = nullptr,
               DocRef::Kind k3 = DocRef::INVALID, const char *name3 = nullptr,
               const char *link3 = nullptr) {
    addDoc(k0, name0, link0);
    addDoc(k1, name1, link1);
    addDoc(k2, name2, link2);
    addDoc(k3, name3, link3);
  }
  const char *chooseDoc(const char *preXe, const char *xe,
                        const char *xe23) const {
    preXe = preXe ? preXe : "?";
    xe = xe ? xe : "?";
    xe23 = xe23 ? xe23 : "?";
    return platform() < Platform::XE    ? preXe
           : platform() < Platform::XE2 ? xe
                                        : xe23;
  }
  void addDocRefDESC(const char *type, const char *xeRef, const char *xe2Ref) {
    if (platform() >= Platform::XE2) {
      addDoc(DocRef::DESC, type, xe2Ref);
    } else if (platform() >= Platform::XE) {
      addDoc(DocRef::DESC, type, xeRef);
    }
  }
  void addDocRefDESCXE2(
      const char *xe2Type, const char *xe2Ref)
  {
    if (platform() >= Platform::XE2) {
      addDoc(DocRef::DESC, xe2Type, xe2Ref);
    }
  }

  /////////////////////////////////////////////////////////////
  // diagnostics
  template <typename T1, typename T2 = const char *, typename T3 = const char *>
  void addDiag(DiagnosticList &dl, int off, int len, T1 t1, T2 t2 = "",
               T3 t3 = "") {
    std::stringstream ss;
    ss << t1 << t2 << t3;
    dl.emplace_back(DescField(off, len), ss.str());
  }
  template <typename T1, typename T2 = const char *, typename T3 = const char *>
  void warning(int off, int len, T1 t1, T2 t2 = "", T3 t3 = "") {
    addDiag(result.warnings, off, len, t1, t2, t3);
  }
  template <typename T1, typename T2 = const char *, typename T3 = const char *>
  void error(int off, int len, T1 t1, T2 t2 = "", T3 t3 = "") {
    addDiag(result.errors, off, len, t1, t2, t3);
  }

  // offset +32 to 64 fetch from exDesc
  // peeks at a field without adding it
  uint32_t getDescBits(int off, int len) const {
    uint32_t bits = desc.imm;
    if (off >= 32) {
      off -= 32;
      bits = exDesc.imm;
    }
    uint32_t mask = len == 32 ? 0xFFFFFFFF : ((1 << len) - 1);
    return (int)((bits >> off) & mask);
  }

  uint32_t getDescBit(int off) const { return getDescBits(off, 1) != 0; }

  uint32_t decodeExDescField(const char *fieldName, int off, int len,
                             DescFieldFormatter fmtMeaning = NO_DECODE) {
    auto val = getDescBits(off + 32, len);
    std::stringstream ss;
    fmtMeaning(ss, val);
    addField(fieldName, off + 32, len, val, ss.str());
    return val;
  }
  uint32_t decodeExDescReserved(int off, int len) {
    return decodeReserved(off + 32, len);
  }
  uint32_t decodeDescField(const char *fieldName, int off, int len,
                           DescFieldFormatter fmtMeaning = NO_DECODE) {
    auto val = getDescBits(off, len);
    std::stringstream ss;
    fmtMeaning(ss, val);
    addField(fieldName, off, len, val, ss.str());
    return val;
  }
  uint32_t decodeDescBitField(const char *fieldName, int off, const char *zero,
                              const char *one) {
    return decodeDescField(
        fieldName, off, 1,
        [&](std::stringstream &ss, uint32_t val) { ss << (val ? one : zero); });
  }
  uint32_t decodeDescBitField(const char *fieldName, int off, const char *one) {
    return decodeDescBitField(fieldName, off, "", one);
  }

  // normally use getDescBitsField, but in cases where you've already
  // decoded, the meaning and just want to record the result
  void addField(const char *fieldName, int off, int len, uint32_t val,
                std::string meaning) {
    Fragment f(fieldName, off, len);
    for (const auto &fvs : result.fields) {
      const auto &f1 = std::get<0>(fvs);
      if (f1.overlaps(f)) {
        // uncomment for debugging
        // std::stringstream ss;
        // ss << "overlapped fields: " << f1.name << " and " << f.name;
        // IGA_ASSERT_FALSE(ss.str().c_str());
        return; // replicated access (don't record again)
      }
    }
    result.fields.emplace_back(f, val, meaning);
  }

  ///////////////////////////////////////////////////////////////////////////
  // decoder helpers
  bool decodeExpected(int off, int len, const char *fieldName,
                      uint32_t expected) {
    auto val = getDescBits(off, len);
    if (val != expected) {
      warning(off, len, "field should be ", expected);
    }
    addField(fieldName, off, len, val, "");
    return val == expected;
  }
  uint32_t decodeReserved(int off, int len) {
    auto val = getDescBits(off, len);
    if (val != 0) {
      warning(off, len, "reserved field should be zero");
    }
    addField("Reserved", off, len, val, "");
    return val;
  }

  // decodes MLen, RLen, and XLen if present
  // (Src0.Length, Dst.Length, Src1.Length)
  void decodePayloadSizes();

  ///////////////////////////////////////////////////////////////////////////
  // the most generic setter
  void
  setScatterGatherOpX(std::string msgSym, std::string msgImpl, SendOp op,
                      AddrType addrType, SendDesc surfaceId, CacheOpt l1,
                      CacheOpt l3, int addrSize, int bitsPerElemReg,
                      int bitsPerElemMem, int elemsPerAddr, int simd,
                      MessageInfo::Attr extraAttrs = MessageInfo::Attr::NONE) {
    MessageInfo &mi = result.info;
    mi.symbol = msgSym;
    mi.description = msgImpl;
    mi.op = op;
    mi.cachingL1 = l1;
    mi.cachingL3 = l3;
    mi.addrType = addrType;
    mi.surfaceId = surfaceId;
    mi.attributeSet |= extraAttrs;
    mi.addrSizeBits = addrSize;
    mi.elemSizeBitsRegFile = bitsPerElemReg;
    mi.elemSizeBitsMemory = bitsPerElemMem;
    mi.elemsPerAddr = elemsPerAddr;
    mi.channelsEnabled = 0;
    mi.execWidth = simd;
  }

  void
  setScatterGatherOp(std::string msgSym, std::string msgDesc, SendOp op,
                     AddrType addrType, SendDesc surfaceId, int addrSize,
                     int bitsPerElem, int elemsPerAddr, int simd,
                     MessageInfo::Attr extraAttrs = MessageInfo::Attr::NONE) {
    setScatterGatherOpX(msgSym, msgDesc, op, addrType, surfaceId,
                        CacheOpt::DEFAULT, CacheOpt::DEFAULT, addrSize,
                        bitsPerElem, bitsPerElem, elemsPerAddr, simd,
                        extraAttrs);
  }

  // for miscellaneous stuff such as fences and whatnot
  //
  // treat the payloads as full register units and set the op to SIMD1
  void setSpecialOpX(std::string msgSym, std::string msgDesc, SendOp op,
                     AddrType addrType, SendDesc surfaceId, int mlen, int rlen,
                     MessageInfo::Attr extraAttrs = MessageInfo::Attr::NONE) {
    MessageInfo &mi = result.info;
    mi.symbol = msgSym;
    mi.description = msgDesc;
    mi.op = op;
    mi.cachingL1 = CacheOpt::DEFAULT;
    mi.cachingL3 = CacheOpt::DEFAULT;
    mi.addrType = addrType;
    mi.surfaceId = surfaceId;
    mi.addrSizeBits = mlen * BITS_PER_REGISTER;
    // e.g. SIMD16 platforms are 256b (two full registers)
    mi.elemSizeBitsRegFile = rlen * BITS_PER_REGISTER;
    mi.elemSizeBitsMemory = mi.elemSizeBitsRegFile;
    mi.channelsEnabled = 0;
    mi.elemsPerAddr = 1;
    mi.execWidth = 1;
    mi.attributeSet = extraAttrs | MessageInfo::Attr::VALID;
  }

  // shared by subclasses
  void addLscFenceFields(std::stringstream &sym, std::stringstream &desc);
  void addLscFenceScopeField(std::stringstream &sym, std::stringstream &desc);
  void addLscFencePortFields(std::stringstream &sym, std::stringstream &desc);
}; // MessageDecoder

///////////////////////////////////////////////////////////////////////////
// shared by MessageDecoderHDC, MessageDecoderOther
struct MessageDecoderLegacy : MessageDecoder {
  static const int SLM_BTI = 0xFE;
  static const int COHERENT_BTI = 0xFF;
  static const int NONCOHERENT_BTI = 0xFD;

  MessageDecoderLegacy(Platform _platform, SFID _sfid, ExecSize _instExecSize,
                       uint32_t exImmOffDesc, SendDesc _exDesc, SendDesc _desc,
                       DecodeResult &_result)
      : MessageDecoder(_platform, _sfid, _instExecSize, exImmOffDesc, _exDesc,
                       _desc, _result) {}

  // from legacy encodings
  int decodeBTI(int addrBits) {
    int bti = (int)getDescBits(0, 8);
    std::stringstream ss;
    ss << "surface " << bti;
    if (bti == SLM_BTI) {
      ss << " (SLM)";
    } else if (bti == COHERENT_BTI) {
      if (addrBits == 64)
        ss << " A64 ";
      else
        ss << " A32 ";
      ss << " (coherent stateless)";
    } else if (bti == NONCOHERENT_BTI) {
      if (addrBits == 64)
        ss << " A64";
      else
        ss << " A32";
      ss << " (incoherent stateless)";
    } else if (bti == 0xFC) {
      ss << " (SSO)";
    }
    addField("BTI", 0, 8, bti, ss.str());
    return bti;
  }

  /////////////////////////////////////////////////////
  // "header" decoding
  bool decodeMDC_H() { // optional
    return decodeDescBitField("Header", 19, "absent", "included") != 0;
  }
  void decodeMDC_HF() {
    if (getDescBit(19) != 0)
      warning(19, 1, "this message forbids a header (and it's included)");
  }
  void decodeMDC_HR() {
    if (!decodeMDC_H())
      warning(19, 1, "this message requires a header (and it's absent)");
  }
  bool decodeMDC_H2() {
    return decodeDescBitField("DualHeader", 19, "absent",
                              "included (two register header)") != 0;
  }

  ///////////////////////////////////////////////////////////////////////
  // some shared decoder helpers
  int decodeMDC_SM2(int off) {
    // yeah SM2 is really 1 bit (2 means two values)
    int bits = decodeDescBitField("SimdMode:MDC_SM2", off, "SIMD8", "SIMD16");
    return bits ? 16 : 8;
  }
}; // MessageDecoderLegacy

// see MessageDecoderHDC.cpp
void decodeDescriptorsHDC(Platform platform, SFID sfid, ExecSize execSize,
                          uint32_t exImmOffDesc, SendDesc exDesc, SendDesc desc,
                          DecodeResult &result);

// see MessageDecoderOther.cpp
void decodeDescriptorsOther(Platform platform, SFID sfid, ExecSize execSize,
                            uint32_t exImmOffDesc, SendDesc exDesc,
                            SendDesc desc, DecodeResult &result);

void decodeDescriptorsLSC(Platform platform, SFID sfid, ExecSize execSize,
                          uint32_t exImmOffDesc, SendDesc exDesc, SendDesc desc,
                          DecodeResult &result);

bool encodeDescriptorsLSC(Platform p, const VectorMessageArgs &vma,
                          uint32_t &exImmOffDesc, SendDesc &exDesc,
                          SendDesc &desc, std::string &err);

// Different parameters that sampler messages allow.
enum class SamplerParam {
  NONE = 0,
  AI,      // array index
  BIAS,    // depth bias
  BIAS_AI, // bias packed with AI
  DUDX,    // u derivative with respect to x
  DUDY,    // u derivative with respect to y
  DUMMY,   // dummy parameter for messages with no arguments
           // sampler send requires src0 to be something
  DVDX,    // v derivative with respect to x
  DVDY,    // v derivative with respect to y
  LOD,     // level of detail
  LOD_AI,  // legel of detail packed with array index
  MCS0,    // multi control surface buffer 0
  MCS1,    // multi control surface buffer 1
  MCS2,    // multi control surface buffer 2
  MCS3,    // multi control surface buffer 3
  MLOD,    // min level of detail
  MLOD_R,  // mlod packed with R
  R,       // r-coordinate (depth)
  REF,     // reference value (_c)
  SI,      // sampler index?
  U,       // u-coordinate
  V,       // v-coordinate
  OFFUV_R,     // r parameter combined with u and v offsets
  OFFUVR,      // u, v, and r offsets combined into a single parameter
  OFFUV,       // u and v offsets combined into a single parameter
  OFFUVR_R,    // r parameter combine with u, v, and r offsets
  BIAS_OFFUVR, // bias parameter and u, v, r offsets combined
  BIAS_OFFUV,  // bias parameter and u, v offsets combined
  LOD_OFFUV,   // LOD parameter and u, v offsets combined
  LOD_OFFUVR,  // LOD parameter and u, v, and r offsets combined
}; // SamplerParam

struct SamplerMessageDescription {
  SendOp op;
  const char *mnemonic;
  int required;
  SamplerParam params[8];

  constexpr SamplerMessageDescription(SendOp o, const char *mne, int reqd,
                                      SamplerParam p0 = SamplerParam::NONE,
                                      SamplerParam p1 = SamplerParam::NONE,
                                      SamplerParam p2 = SamplerParam::NONE,
                                      SamplerParam p3 = SamplerParam::NONE,
                                      SamplerParam p4 = SamplerParam::NONE,
                                      SamplerParam p5 = SamplerParam::NONE,
                                      SamplerParam p6 = SamplerParam::NONE,
                                      SamplerParam p7 = SamplerParam::NONE)
      : op(o), mnemonic(mne),
        required(reqd), params{p0, p1, p2, p3, p4, p5, p6, p7} {}

  int countParams() const;

  std::string describe(int srcsLen) const;
}; // SamplerMessageDescription

std::string ToSymbol(SamplerParam sp);
} // namespace iga

#endif // _IGA_BACKEND_MESSAGES_MESSAGEDECODER_HPP_
