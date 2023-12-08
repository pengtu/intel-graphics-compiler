;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2023 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================
;
; RUN: igc_opt -enable-debugify -igc-joint-matrix-resolution -dce -S --platformdg2 2>&1 < %s | FileCheck %s
; ------------------------------------------------
; JointMatrixFuncsResolutionPass
; ------------------------------------------------


%intel.joint_matrix_packedA_8x16_i32_ = type opaque

define spir_kernel void @load_store_legacy_error(i8* %a, i8* %dst) {
; CHECK-LABEL: define spir_kernel void @load_store_legacy_error(
; CHECK: [[PTR:%.*]] = alloca <8 x i32>
; CHECK: [[MATPTR:%.*]] = bitcast <8 x i32>* [[PTR]] to i8*
; CHECK: call void @__builtin_spriv_OpJointMatrixLoadINTEL_PackedA_RowMajor_8x16_i32_8_generic_v8i8_pi32_i32(i8* [[MATPTR]], i8* %a, i32 16), !dbg [[DBG2:![0-9]*]]
; CHECK: [[MATRIX:%.*]] = load <8 x i32>, <8 x i32>* [[PTR]]
; CHECK: [[TMP4:%.*]] = alloca <8 x i32>
; CHECK: store <8 x i32> [[MATRIX]], <8 x i32>* [[TMP4]]
; CHECK: [[TMP5:%.*]] = bitcast <8 x i32>* [[TMP4]] to i8*
; CHECK: call void @__builtin_spriv_OpJointMatrixStoreINTEL_PackedA_RowMajor_8x16_i32_8_generic_pi64_v8i8(i8* %dst, i8* [[TMP5]], i32 8), !dbg [[DBG3:![0-9]*]]
; CHECK: ret void
; CHECK: error: Unsupported JointMatrix operation: load matrix A <8 x 16 x i32> with row major layout
; CHECK: error: Unsupported JointMatrix operation: store matrix A <8 x 16 x i32> with row major layout
;
  %1 = call spir_func %intel.joint_matrix_packedA_8x16_i32_* @__builtin_spirv_OpJointMatrixLoadINTEL(i8* %a, i32 16, i32 0)
  call spir_func void @__builtin_spirv_OpJointMatrixStoreINTEL.8x16(i8* %dst, %intel.joint_matrix_packedA_8x16_i32_* %1, i32 8, i32 0)
  ret void
}

declare spir_func %intel.joint_matrix_packedA_8x16_i32_* @__builtin_spirv_OpJointMatrixLoadINTEL(i8*, i32, i32)
declare spir_func void @__builtin_spirv_OpJointMatrixStoreINTEL.8x16(i8*, %intel.joint_matrix_packedA_8x16_i32_*, i32, i32)

!igc.functions = !{!0}
!0 = !{void (i8*, i8*)* @load_store_legacy_error, !1}
!1 = !{!2, !3}
!2 = !{!"function_type", i32 0}
!3 = !{!"sub_group_size", i32 8}
