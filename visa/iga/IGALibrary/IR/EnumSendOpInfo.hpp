/*========================== begin_copyright_notice ============================

Copyright (C) 2020-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

//////////////////////////////////////////////////////////////////////////
// Creates an enumerated list of ops via macro callback.
// This keeps the op table stable as we add and remove elements
//
// Formally, this is transposing the Expression Problem:
//  + adding new variants is easy (1 line)
//  - adding new attributes/properties requires potentially
//    modifying multiple variants (macro cleverness with defaults reduces
//    some of that sting)
#ifndef DEFINE_SEND_OP
#error "DEFINE_SEND_OP(ENUM, MNMEONIC, DESCRIPTION, ATTRS) not defined"
// #define DEFINE_SEND_OP(E, M, D, A)
#endif

#ifndef ATTRS_NONE
// in case of recursive additions
#define ATTRS_NONE iga::SendOpDefinition::Attr::NONE
#define ATTRS_SCALARADDR iga::SendOpDefinition::Attr::IS_SCALAR_ADDR
#define ATTRS_CMASK iga::SendOpDefinition::Attr::HAS_CMASK
#define ATTRS_ATOMIC_UNR                                                       \
  iga::SendOpDefinition::Attr::GROUP_ATOMIC |                                  \
      iga::SendOpDefinition::Attr::ATOMIC_UNARY
#define ATTRS_ATOMIC_BIN                                                       \
  iga::SendOpDefinition::Attr::GROUP_ATOMIC |                                  \
      iga::SendOpDefinition::Attr::ATOMIC_BINARY
#define ATTRS_ATOMIC_TER                                                       \
  iga::SendOpDefinition::Attr::GROUP_ATOMIC |                                  \
      iga::SendOpDefinition::Attr::ATOMIC_TERNARY
#define ATTRS_GATHER4 iga::SendOpDefinition::Attr::GATHER4
#endif

#ifndef DEFINE_LOAD_OP
// users can override this if needed
#define DEFINE_LOAD_OP(E, M, D, A)                                             \
  DEFINE_SEND_OP(E, M, D, (A) | iga::SendOpDefinition::Attr::GROUP_LOAD)
#endif
#ifndef DEFINE_STORE_OP
#define DEFINE_STORE_OP(E, M, D, A)                                            \
  DEFINE_SEND_OP(E, M, D, (A) | iga::SendOpDefinition::Attr::GROUP_STORE)
#endif
#ifndef DEFINE_ATOMIC_UNARY_OP
#define DEFINE_ATOMIC_UNARY_OP(E, M, D)                                        \
  DEFINE_SEND_OP(E, M, D, ATTRS_ATOMIC_UNR)
#endif
#ifndef DEFINE_ATOMIC_BINARY_OP
#define DEFINE_ATOMIC_BINARY_OP(E, M, D)                                       \
  DEFINE_SEND_OP(E, M, D, ATTRS_ATOMIC_BIN)
#endif
#ifndef DEFINE_ATOMIC_TERNARY_OP
#define DEFINE_ATOMIC_TERNARY_OP(E, M, D)                                      \
  DEFINE_SEND_OP(E, M, D, ATTRS_ATOMIC_TER)
#endif
#ifndef DEFINE_SAMPLE_OP
#define DEFINE_SAMPLE_OP(E, M, D, A)                                           \
  DEFINE_SEND_OP(E, M, D, (A) | iga::SendOpDefinition::Attr::GROUP_SAMPLE)
#endif
#ifndef DEFINE_OTHER_OP
#define DEFINE_OTHER_OP(E, M, D, A)                                            \
  DEFINE_SEND_OP(E, M, D, (A) | iga::SendOpDefinition::Attr::GROUP_OTHER)
#endif

DEFINE_LOAD_OP(LOAD, "load", "load", ATTRS_NONE)
DEFINE_LOAD_OP(LOAD_STRIDED, "load_strided", "load strided",
               ATTRS_SCALARADDR) // AKA load_block
DEFINE_LOAD_OP(LOAD_QUAD, "load_quad", "load quad",
               ATTRS_CMASK) // AKA load_cmask
DEFINE_LOAD_OP(LOAD_STATUS, "load_status", "load status", ATTRS_NONE)
DEFINE_LOAD_OP(LOAD_BLOCK2D, "load_block2d", "load array of 2d blocks",
               ATTRS_SCALARADDR)
DEFINE_STORE_OP(STORE, "store", "store", ATTRS_NONE)
DEFINE_STORE_OP(STORE_STRIDED, "store_strided", "store strided",
                ATTRS_SCALARADDR) // AKA store_block
DEFINE_STORE_OP(STORE_QUAD, "store_quad", "store quad",
                ATTRS_CMASK) // AKA store_cmask

DEFINE_STORE_OP(STORE_BLOCK2D, "store_block2d", "store 2d block",
                ATTRS_SCALARADDR)
DEFINE_STORE_OP(STORE_UNCOMPRESSED, "store_uncompressed", "uncompressed store",
                ATTRS_NONE)
DEFINE_STORE_OP(STORE_UNCOMPRESSED_QUAD, "store_uncompressed_quad",
                "uncompressed quad store", ATTRS_CMASK)
DEFINE_ATOMIC_UNARY_OP(ATOMIC_LOAD, "atomic_load", "atomic load")
DEFINE_ATOMIC_BINARY_OP(ATOMIC_STORE, "atomic_store", "atomic store")
//
DEFINE_ATOMIC_BINARY_OP(ATOMIC_AND, "atomic_and", "atomic logical AND")
DEFINE_ATOMIC_BINARY_OP(ATOMIC_XOR, "atomic_xor", "atomic logical XOR")
DEFINE_ATOMIC_BINARY_OP(ATOMIC_OR, "atomic_or", "atomic logical OR")
//
DEFINE_ATOMIC_UNARY_OP(ATOMIC_IINC, "atomic_iinc", "atomic integer increment")
DEFINE_ATOMIC_UNARY_OP(ATOMIC_IDEC, "atomic_idec", "atomic integer decrement")
DEFINE_ATOMIC_UNARY_OP(ATOMIC_IPDEC, "atomic_ipdec",
                       "atomic integer pre-decrement")
DEFINE_ATOMIC_BINARY_OP(ATOMIC_IADD, "atomic_iadd", "atomic integer add")
DEFINE_ATOMIC_BINARY_OP(ATOMIC_ISUB, "atomic_isub", "atomic integer subtract")
DEFINE_ATOMIC_BINARY_OP(ATOMIC_IRSUB, "atomic_irsub",
                        "atomic integer reverse subtract")
DEFINE_ATOMIC_TERNARY_OP(ATOMIC_ICAS, "atomic_icas",
                         "atomic integer compare and swap")

DEFINE_ATOMIC_BINARY_OP(ATOMIC_SMIN, "atomic_smin", "atomic signed minimum")
DEFINE_ATOMIC_BINARY_OP(ATOMIC_SMAX, "atomic_smax", "atomic signed maximum")
//
DEFINE_ATOMIC_BINARY_OP(ATOMIC_UMIN, "atomic_umin", "atomic unsigned minimum")
DEFINE_ATOMIC_BINARY_OP(ATOMIC_UMAX, "atomic_umax", "atomic unsigned maximum")
//
DEFINE_ATOMIC_BINARY_OP(ATOMIC_FADD, "atomic_fadd", "atomic floating-point add")
DEFINE_ATOMIC_BINARY_OP(ATOMIC_FSUB, "atomic_fsub",
                        "atomic floating-point subtract")
DEFINE_ATOMIC_BINARY_OP(ATOMIC_FMIN, "atomic_fmin",
                        "atomic floating-point minimum")
DEFINE_ATOMIC_BINARY_OP(ATOMIC_FMAX, "atomic_fmax",
                        "atomic floating-point maximum")
DEFINE_ATOMIC_TERNARY_OP(ATOMIC_FCAS, "atomic_fcas",
                         "atomic floating-point compare and swap")
//
// for now use DEFINE_OTHER_OP to prevent syntax
DEFINE_OTHER_OP(ATOMIC_ACADD, "atomic_acadd", "atomic counter add",
                iga::SendOpDefinition::Attr::GROUP_ATOMIC |
                    iga::SendOpDefinition::Attr::ATOMIC_BINARY)
DEFINE_OTHER_OP(ATOMIC_ACSUB, "atomic_acsub", "atomic counter subtract",
                iga::SendOpDefinition::Attr::GROUP_ATOMIC |
                    iga::SendOpDefinition::Attr::ATOMIC_BINARY)
DEFINE_OTHER_OP(ATOMIC_ACSTORE, "atomic_acstore", "atomic counter store",
                iga::SendOpDefinition::Attr::GROUP_ATOMIC |
                    iga::SendOpDefinition::Attr::ATOMIC_BINARY)
//
DEFINE_OTHER_OP(READ_STATE, "read_state", "read surface state", ATTRS_NONE)
//
DEFINE_OTHER_OP(FENCE, "fence", "fence", ATTRS_NONE)
//
DEFINE_OTHER_OP(SIGNAL_BARRIER, "signal_barrier", "signal barrier", ATTRS_NONE)
DEFINE_OTHER_OP(SIGNAL_NAMED_BARRIER, "signal_named_barrier", "signal named barrier",
                ATTRS_NONE)
DEFINE_OTHER_OP(SIGNAL_SYSTEM_ROUTINE_BARRIER, "system_routine_barrier",
                "system routine barrier", ATTRS_NONE)
DEFINE_OTHER_OP(MONITOR, "monitor", "monitor event", ATTRS_NONE)
DEFINE_OTHER_OP(UNMONITOR, "unmonitor", "unmonitor event", ATTRS_NONE)
DEFINE_OTHER_OP(WAIT, "wait", "wait for event", ATTRS_NONE)
DEFINE_OTHER_OP(SIGNAL, "signal", "signal event", ATTRS_NONE)
DEFINE_OTHER_OP(EOT, "eot", "end of thread", ATTRS_NONE)
DEFINE_OTHER_OP(EOTR, "eotr", "end of thread replay", ATTRS_NONE)
//
DEFINE_OTHER_OP(CCS_PC, "ccs_pc", "ccs page clear", ATTRS_NONE)
DEFINE_OTHER_OP(CCS_PU, "ccs_pu", "ccs page uncompress", ATTRS_NONE)
DEFINE_OTHER_OP(CCS_SC, "ccs_sc", "ccs sector clear", ATTRS_CMASK)
DEFINE_OTHER_OP(CCS_SU, "ccs_su", "ccs sector uncompress", ATTRS_CMASK)
//
DEFINE_OTHER_OP(TRACE_RAY, "trace_ray", "trace ray", ATTRS_NONE)
DEFINE_OTHER_OP(SPAWN, "spawn", "spawn thread", ATTRS_NONE)
//
DEFINE_SAMPLE_OP(SAMPLE, "sample", "sampler fetch", ATTRS_NONE)
DEFINE_SAMPLE_OP(SAMPLE_B, "sample_b", "sample with bias", ATTRS_NONE)
DEFINE_SAMPLE_OP(SAMPLE_L, "sample_l", "sample with LoD", ATTRS_NONE)
DEFINE_SAMPLE_OP(SAMPLE_C, "sample_c", "sample with compare", ATTRS_NONE)
DEFINE_SAMPLE_OP(SAMPLE_D, "sample_d", "sample with derivatives", ATTRS_NONE)
DEFINE_SAMPLE_OP(SAMPLE_B_C, "sample_b_c", "sample with bias and compare",
                 ATTRS_NONE)
DEFINE_SAMPLE_OP(SAMPLE_L_C, "sample_l_c",
                 "sample with LoD and compare", ATTRS_NONE)
DEFINE_SAMPLE_OP(SAMPLE_D_C, "sample_d_c",
                 "sampler with derivatives and compare", ATTRS_NONE)
DEFINE_SAMPLE_OP(SAMPLE_LZ, "sample_lz", "sampler lz", ATTRS_NONE)
DEFINE_SAMPLE_OP(SAMPLE_C_LZ, "sample_c_lz",
                 "sampler with compare and lz", ATTRS_NONE)
//
DEFINE_SAMPLE_OP(LD, "ld", "sampler load", ATTRS_NONE)
DEFINE_SAMPLE_OP(SAMPLE_LOD, "sample_lod", "sampler LoD", ATTRS_NONE)
DEFINE_SAMPLE_OP(LD_LZ, "ld_lz", "sampler ld lz", ATTRS_NONE)
DEFINE_SAMPLE_OP(SAMPLE_RESINFO, "resinfo", "sampler resolution info",
                 ATTRS_NONE)
DEFINE_SAMPLE_OP(SAMPLE_INFO, "sample_info", "sampler info", ATTRS_NONE)
DEFINE_SAMPLE_OP(SAMPLE_KILLPIX, "sample_killpix", "sampler kill pixel",
                 ATTRS_NONE)
DEFINE_SAMPLE_OP(LD_2DMS_W, "ld_2dms_w", "ld mcs4", ATTRS_NONE)
DEFINE_SAMPLE_OP(LD_MCS, "ld_mcs", "load MCS", ATTRS_NONE)
DEFINE_SAMPLE_OP(SAMPLE_FLUSH, "sample_flush",
                 "sample with UVR offsets and compare", ATTRS_NONE)
DEFINE_SAMPLE_OP(SAMPLE_MLOD, "sample_mlod", "sample with LoD", ATTRS_NONE)
DEFINE_SAMPLE_OP(SAMPLE_C_MLOD, "sample_c_mlod",
                 "sample with mlod and compare", ATTRS_NONE)
DEFINE_SAMPLE_OP(SAMPLE_D_C_MLOD, "sample_d_c_mlod",
                 "sampler with mlod, derivatives, and compare", ATTRS_NONE)
//
DEFINE_SAMPLE_OP(SAMPLE_PO, "sample_po", "sample with UVR offsets", ATTRS_NONE)
DEFINE_SAMPLE_OP(SAMPLE_PO_B, "sample_po_b", "sample with UVR offsets and bias",
                 ATTRS_NONE)
DEFINE_SAMPLE_OP(SAMPLE_PO_L, "sample_po_l", "sample with pixel offset and LoD",
                 ATTRS_NONE)
DEFINE_SAMPLE_OP(SAMPLE_PO_C, "sample_po_c",
                 "sample with UVR offsets and compare", ATTRS_NONE)
DEFINE_SAMPLE_OP(SAMPLE_PO_D, "sample_po_d",
                 "sample with derivatives and offsets", ATTRS_NONE)
DEFINE_SAMPLE_OP(SAMPLE_PO_L_C, "sample_po_l_c",
                 "sample with offsets, LoD, and compare", ATTRS_NONE)
DEFINE_SAMPLE_OP(SAMPLE_PO_LZ, "sample_po_lz", "sampler lz with pos. offset", ATTRS_NONE)
DEFINE_SAMPLE_OP(SAMPLE_PO_C_LZ, "sample_po_c_lz", "sampler pos. offset compare ld", ATTRS_NONE)
//
DEFINE_SAMPLE_OP(SAMPLE_LD_L, "sample_ld_l", "sampler ld LoD", ATTRS_NONE)
//
DEFINE_SAMPLE_OP(GATHER4, "gather4", "gather4", ATTRS_GATHER4)
DEFINE_SAMPLE_OP(GATHER4_C, "gather4_c", "gather4 with compare",
                 ATTRS_GATHER4)
DEFINE_SAMPLE_OP(GATHER4_I, "gather4_i", "gather4 with index", ATTRS_GATHER4)
DEFINE_SAMPLE_OP(GATHER4_I_C, "gather4_i_c", "gather4 with offsets and LoD",
                 ATTRS_GATHER4)
DEFINE_SAMPLE_OP(GATHER4_B, "gather4_b", "gather4 with bias", ATTRS_GATHER4)
DEFINE_SAMPLE_OP(GATHER4_L, "gather4_l", "gather4 with LoD", ATTRS_GATHER4)
DEFINE_SAMPLE_OP(GATHER4_L_C, "gather4_l_c",
                 "gather4 with offsets and LoD, and compare",
                 ATTRS_GATHER4)
//
DEFINE_SAMPLE_OP(GATHER4_PO, "gather4_po", "gather4 with offsets",
                 ATTRS_GATHER4)
DEFINE_SAMPLE_OP(GATHER4_PO_L, "gather4_po_l", "gather4 with offsets and LoD",
                 ATTRS_GATHER4)
DEFINE_SAMPLE_OP(GATHER4_PO_C, "gather4_po_c", "gather4 with offsets and compare",
                 ATTRS_GATHER4)
DEFINE_SAMPLE_OP(GATHER4_PO_B, "gather4_po_b", "gather4 with offsets and LoD",
                 ATTRS_GATHER4)
DEFINE_SAMPLE_OP(GATHER4_PO_I, "gather4_po_i", "gather4 with offsets and index",
                 ATTRS_GATHER4)
DEFINE_SAMPLE_OP(GATHER4_PO_I_C, "gather4_po_i_c",
                 "gather4 with offsets, index, and compare",
                 ATTRS_GATHER4)
DEFINE_SAMPLE_OP(GATHER4_PO_L_C, "gather4_po_l_c",
                 "gather4 with offsets, LoD and compare",
                 ATTRS_GATHER4)
//
DEFINE_OTHER_OP(RENDER_READ, "render_read", "render read", ATTRS_NONE)
DEFINE_OTHER_OP(RENDER_WRITE, "render_write", "render write", ATTRS_NONE)
DEFINE_OTHER_OP(RENDER_DWRITE, "render_dual_write", "render dual write", ATTRS_NONE)

DEFINE_OTHER_OP(SAVE_BARRIER, "save_barrier", "save barrier", ATTRS_NONE)
DEFINE_OTHER_OP(RESTORE_BARRIER, "restore_barrier", "restore barrier",
                ATTRS_NONE)
DEFINE_OTHER_OP(RESTORE_STACK, "restore_stack", "restore stack", ATTRS_NONE)
//
//

#undef ATTRS_NONE
#undef ATTRS_SCALARADDR
#undef ATTRS_CMASK
#undef ATTRS_GATHER4
