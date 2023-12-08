;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2023 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================
;
; RUN: igc_opt -igc-joint-matrix-resolution -dce -S 2>&1 < %s | FileCheck %s
; ------------------------------------------------
; JointMatrixFuncsResolutionPass
; ------------------------------------------------

%intel.joint_matrix_packedA_8x16_i32_t = type opaque
%intel.joint_matrix_acc_32x64_f32_t = type opaque

define spir_kernel void @test_generic(i8* %src, i8* %dst) {
  call void @load_store_generic(i8* %src, i8* %dst)
  call void @load_store_large_generic(i8* %src, i8* %dst)
  ret void
}

define spir_kernel void @test_global(i8 addrspace(1)* %src, i8 addrspace(1)* %dst) {
  call void @load_store_global(i8 addrspace(1)* %src, i8 addrspace(1)* %dst)
  call void @load_store_large_global(i8 addrspace(1)* %src, i8 addrspace(1)* %dst)
  ret void
}

define spir_kernel void @test_local(i8 addrspace(3)*  %src, i8 addrspace(3)* %dst) {
  call void @load_store_local(i8 addrspace(3)* %src, i8 addrspace(3)* %dst)
  call void @load_store_large_local(i8 addrspace(3)* %src, i8 addrspace(3)* %dst)
  ret void
}

; CHECK-LABEL: define void @load_store_generic(
define void @load_store_generic(i8* %src, i8* %dst) {

; Matrix load sequence:
; CHECK: [[PTR:%.*]] = alloca <8 x i32>
; CHECK: [[MATPTR:%.*]] = bitcast <8 x i32>* [[PTR]] to i8*
; CHECK: call void @__builtin_spriv_OpJointMatrixLoadINTEL_PackedA_RowMajor_8x16_i32_8_generic_v8i8_pi32_i32(i8* [[MATPTR]], i8* %src, i32 16)
; CHECK: [[MATRIX:%.*]] = load <8 x i32>, <8 x i32>* [[PTR]]

  %1 = call spir_func %intel.joint_matrix_packedA_8x16_i32_t* @__builtin_spirv_OpJointMatrixLoadINTEL_generic(i8* %src, i32 16, i32 0)

; Matrix store sequence:
; CHECK: [[TMP4:%.*]] = alloca <8 x i32>
; CHECK: store <8 x i32> [[MATRIX]], <8 x i32>* [[TMP4]]
; CHECK: [[TMP5:%.*]] = bitcast <8 x i32>* [[TMP4]] to i8*
; CHECK: call void @__builtin_spriv_OpJointMatrixStoreINTEL_PackedA_RowMajor_8x16_i32_8_generic_pi64_v8i8(i8* %dst, i8* [[TMP5]], i32 8)

  call spir_func void @__builtin_spirv_OpJointMatrixStoreINTEL.8x16_generic(i8* %dst, %intel.joint_matrix_packedA_8x16_i32_t* %1, i32 8, i32 0)

; CHECK: ret void

  ret void
}

; CHECK-LABEL: define void @load_store_large_generic(
define void @load_store_large_generic(i8* %src, i8* %dst) {

; Matrix load sequence:
; CHECK: [[PTR:%.*]] = alloca [2 x <32 x i64>]
; CHECK: [[MATPTR:%.*]] = bitcast [2 x <32 x i64>]* [[PTR]] to i8*
; CHECK: call void @__builtin_spriv_OpJointMatrixLoadINTEL_Accumulator_RowMajor_32x64_i32_32_generic_v8i8_pi32_i32(i8* [[MATPTR]], i8* %src, i64 16)
; CHECK: [[HALF_PTR_0:%.*]] = bitcast [2 x <32 x i64>]* [[PTR]] to <32 x i64>*
; CHECK: [[HALF_VAL_0:%.*]] = load <32 x i64>, <32 x i64>* [[HALF_PTR_0]]
; CHECK: [[HALF_PTR_1:%.*]] = getelementptr <32 x i64>, <32 x i64>* [[HALF_PTR_0]], i32 1
; CHECK: [[HALF_VAL_1:%.*]] = load <32 x i64>, <32 x i64>* [[HALF_PTR_1]]
; CHECK: [[MATRIX_PARTIAL:%.*]] = insertvalue [2 x <32 x i64>] undef, <32 x i64> [[HALF_VAL_0]], 0
; CHECK: [[MATRIX:%.*]] = insertvalue [2 x <32 x i64>] [[MATRIX_PARTIAL]], <32 x i64> [[HALF_VAL_1]], 1

  %1 = call spir_func %intel.joint_matrix_acc_32x64_f32_t* @__builtin_spirv_OpJointMatrixLoadINTELacc_32x64_f32_p1i8_i64_i32_generic(i8* %src, i64 16, i32 0)

; Matrix store sequence:
; CHECK: [[TMP4:%.*]] = alloca [2 x <32 x i64>]
; CHECK: store [2 x <32 x i64>] [[MATRIX]], [2 x <32 x i64>]* [[TMP4]]
; CHECK: [[TMP5:%.*]] = bitcast [2 x <32 x i64>]* [[TMP4]] to i8*
; CHECK: call void @__builtin_spriv_OpJointMatrixStoreINTEL_Accumulator_RowMajor_32x64_i32_32_generic_pi64_v8i8(i8* %dst, i8* [[TMP5]], i64 8)

  call spir_func void @__builtin_spirv_OpJointMatrixStoreINTELacc_32x64_f32_p1i8_acc_32x64_f32_i64_i32_generic(i8* %dst, %intel.joint_matrix_acc_32x64_f32_t* %1, i64 8, i32 0)

; CHECK: ret void

  ret void
}

; CHECK-LABEL: define void @load_store_global(
define void @load_store_global(i8 addrspace(1)* %src, i8 addrspace(1)* %dst) {

; Matrix load sequence:
; CHECK: [[PTR:%.*]] = alloca <8 x i32>
; CHECK: [[MATPTR:%.*]] = bitcast <8 x i32>* [[PTR]] to i8*
; CHECK: call void @__builtin_spriv_OpJointMatrixLoadINTEL_PackedA_RowMajor_8x16_i32_8_global_v8i8_pi32_i32(i8* [[MATPTR]], i8 addrspace(1)* %src, i32 16)
; CHECK: [[MATRIX:%.*]] = load <8 x i32>, <8 x i32>* [[PTR]]

  %1 = call spir_func %intel.joint_matrix_packedA_8x16_i32_t* @__builtin_spirv_OpJointMatrixLoadINTEL_global(i8 addrspace(1)* %src, i32 16, i32 0)

; Matrix store sequence:
; CHECK: [[TMP4:%.*]] = alloca <8 x i32>
; CHECK: store <8 x i32> [[MATRIX]], <8 x i32>* [[TMP4]]
; CHECK: [[TMP5:%.*]] = bitcast <8 x i32>* [[TMP4]] to i8*
; CHECK: call void @__builtin_spriv_OpJointMatrixStoreINTEL_PackedA_RowMajor_8x16_i32_8_global_pi64_v8i8(i8 addrspace(1)* %dst, i8* [[TMP5]], i32 8)

  call spir_func void @__builtin_spirv_OpJointMatrixStoreINTEL.8x16_global(i8 addrspace(1)* %dst, %intel.joint_matrix_packedA_8x16_i32_t* %1, i32 8, i32 0)

; CHECK: ret void

  ret void
}

; CHECK-LABEL: define void @load_store_large_global(
define void @load_store_large_global(i8 addrspace(1)* %src, i8 addrspace(1)* %dst) {

; Matrix load sequence:
; CHECK: [[PTR:%.*]] = alloca [2 x <32 x i64>]
; CHECK: [[MATPTR:%.*]] = bitcast [2 x <32 x i64>]* [[PTR]] to i8*
; CHECK: call void @__builtin_spriv_OpJointMatrixLoadINTEL_Accumulator_RowMajor_32x64_i32_32_global_v8i8_pi32_i32(i8* [[MATPTR]], i8 addrspace(1)* %src, i64 16)
; CHECK: [[HALF_PTR_0:%.*]] = bitcast [2 x <32 x i64>]* [[PTR]] to <32 x i64>*
; CHECK: [[HALF_VAL_0:%.*]] = load <32 x i64>, <32 x i64>* [[HALF_PTR_0]]
; CHECK: [[HALF_PTR_1:%.*]] = getelementptr <32 x i64>, <32 x i64>* [[HALF_PTR_0]], i32 1
; CHECK: [[HALF_VAL_1:%.*]] = load <32 x i64>, <32 x i64>* [[HALF_PTR_1]]
; CHECK: [[MATRIX_PARTIAL:%.*]] = insertvalue [2 x <32 x i64>] undef, <32 x i64> [[HALF_VAL_0]], 0
; CHECK: [[MATRIX:%.*]] = insertvalue [2 x <32 x i64>] [[MATRIX_PARTIAL]], <32 x i64> [[HALF_VAL_1]], 1

  %1 = call spir_func %intel.joint_matrix_acc_32x64_f32_t* @__builtin_spirv_OpJointMatrixLoadINTELacc_32x64_f32_p1i8_i64_i32_global(i8 addrspace(1)* %src, i64 16, i32 0)

; Matrix store sequence:
; CHECK: [[TMP4:%.*]] = alloca [2 x <32 x i64>]
; CHECK: store [2 x <32 x i64>] [[MATRIX]], [2 x <32 x i64>]* [[TMP4]]
; CHECK: [[TMP5:%.*]] = bitcast [2 x <32 x i64>]* [[TMP4]] to i8*
; CHECK: call void @__builtin_spriv_OpJointMatrixStoreINTEL_Accumulator_RowMajor_32x64_i32_32_global_pi64_v8i8(i8 addrspace(1)* %dst, i8* [[TMP5]], i64 8)

  call spir_func void @__builtin_spirv_OpJointMatrixStoreINTELacc_32x64_f32_p1i8_acc_32x64_f32_i64_i32_global(i8 addrspace(1)* %dst, %intel.joint_matrix_acc_32x64_f32_t* %1, i64 8, i32 0)

; CHECK: ret void

  ret void
}

; CHECK-LABEL: define void @load_store_local(
define void @load_store_local(i8 addrspace(3)* %src, i8 addrspace(3)* %dst) {

; Matrix load sequence:
; CHECK: [[PTR:%.*]] = alloca <8 x i32>
; CHECK: [[MATPTR:%.*]] = bitcast <8 x i32>* [[PTR]] to i8*
; CHECK: call void @__builtin_spriv_OpJointMatrixLoadINTEL_PackedA_RowMajor_8x16_i32_8_local_v8i8_pi32_i32(i8* [[MATPTR]], i8 addrspace(3)* %src, i32 16)
; CHECK: [[MATRIX:%.*]] = load <8 x i32>, <8 x i32>* [[PTR]]

  %1 = call spir_func %intel.joint_matrix_packedA_8x16_i32_t* @__builtin_spirv_OpJointMatrixLoadINTEL_local(i8 addrspace(3)* %src, i32 16, i32 0)

; Matrix store sequence:
; CHECK: [[TMP4:%.*]] = alloca <8 x i32>
; CHECK: store <8 x i32> [[MATRIX]], <8 x i32>* [[TMP4]]
; CHECK: [[TMP5:%.*]] = bitcast <8 x i32>* [[TMP4]] to i8*
; CHECK: call void @__builtin_spriv_OpJointMatrixStoreINTEL_PackedA_RowMajor_8x16_i32_8_local_pi64_v8i8(i8 addrspace(3)* %dst, i8* [[TMP5]], i32 8)

  call spir_func void @__builtin_spirv_OpJointMatrixStoreINTEL.8x16_local(i8 addrspace(3)* %dst, %intel.joint_matrix_packedA_8x16_i32_t* %1, i32 8, i32 0)

; CHECK: ret void

  ret void
}

; CHECK-LABEL: define void @load_store_large_local(
define void @load_store_large_local(i8 addrspace(3)* %src, i8 addrspace(3)* %dst) {

; Matrix load sequence:
; CHECK: [[PTR:%.*]] = alloca [2 x <32 x i64>]
; CHECK: [[MATPTR:%.*]] = bitcast [2 x <32 x i64>]* [[PTR]] to i8*
; CHECK: call void @__builtin_spriv_OpJointMatrixLoadINTEL_Accumulator_RowMajor_32x64_i32_32_local_v8i8_pi32_i32(i8* [[MATPTR]], i8 addrspace(3)* %src, i64 16)
; CHECK: [[HALF_PTR_0:%.*]] = bitcast [2 x <32 x i64>]* [[PTR]] to <32 x i64>*
; CHECK: [[HALF_VAL_0:%.*]] = load <32 x i64>, <32 x i64>* [[HALF_PTR_0]]
; CHECK: [[HALF_PTR_1:%.*]] = getelementptr <32 x i64>, <32 x i64>* [[HALF_PTR_0]], i32 1
; CHECK: [[HALF_VAL_1:%.*]] = load <32 x i64>, <32 x i64>* [[HALF_PTR_1]]
; CHECK: [[MATRIX_PARTIAL:%.*]] = insertvalue [2 x <32 x i64>] undef, <32 x i64> [[HALF_VAL_0]], 0
; CHECK: [[MATRIX:%.*]] = insertvalue [2 x <32 x i64>] [[MATRIX_PARTIAL]], <32 x i64> [[HALF_VAL_1]], 1

  %1 = call spir_func %intel.joint_matrix_acc_32x64_f32_t* @__builtin_spirv_OpJointMatrixLoadINTELacc_32x64_f32_p1i8_i64_i32_local(i8 addrspace(3)* %src, i64 16, i32 0)

; Matrix store sequence:
; CHECK: [[TMP4:%.*]] = alloca [2 x <32 x i64>]
; CHECK: store [2 x <32 x i64>] [[MATRIX]], [2 x <32 x i64>]* [[TMP4]]
; CHECK: [[TMP5:%.*]] = bitcast [2 x <32 x i64>]* [[TMP4]] to i8*
; CHECK: call void @__builtin_spriv_OpJointMatrixStoreINTEL_Accumulator_RowMajor_32x64_i32_32_local_pi64_v8i8(i8 addrspace(3)* %dst, i8* [[TMP5]], i64 8)

  call spir_func void @__builtin_spirv_OpJointMatrixStoreINTELacc_32x64_f32_p1i8_acc_32x64_f32_i64_i32_local(i8 addrspace(3)* %dst, %intel.joint_matrix_acc_32x64_f32_t* %1, i64 8, i32 0)

; CHECK: ret void

  ret void
}

declare spir_func %intel.joint_matrix_packedA_8x16_i32_t* @__builtin_spirv_OpJointMatrixLoadINTEL_generic(i8*, i32, i32)
declare spir_func %intel.joint_matrix_packedA_8x16_i32_t* @__builtin_spirv_OpJointMatrixLoadINTEL_global(i8 addrspace(1)*, i32, i32)
declare spir_func %intel.joint_matrix_packedA_8x16_i32_t* @__builtin_spirv_OpJointMatrixLoadINTEL_local(i8 addrspace(3)*, i32, i32)
declare spir_func void @__builtin_spirv_OpJointMatrixStoreINTEL.8x16_generic(i8*, %intel.joint_matrix_packedA_8x16_i32_t*, i32, i32)
declare spir_func void @__builtin_spirv_OpJointMatrixStoreINTEL.8x16_global(i8 addrspace(1)*, %intel.joint_matrix_packedA_8x16_i32_t*, i32, i32)
declare spir_func void @__builtin_spirv_OpJointMatrixStoreINTEL.8x16_local(i8 addrspace(3)*, %intel.joint_matrix_packedA_8x16_i32_t*, i32, i32)

declare %intel.joint_matrix_acc_32x64_f32_t* @__builtin_spirv_OpJointMatrixLoadINTELacc_32x64_f32_p1i8_i64_i32_generic(i8*, i64, i32)
declare %intel.joint_matrix_acc_32x64_f32_t* @__builtin_spirv_OpJointMatrixLoadINTELacc_32x64_f32_p1i8_i64_i32_global(i8 addrspace(1)*, i64, i32)
declare %intel.joint_matrix_acc_32x64_f32_t* @__builtin_spirv_OpJointMatrixLoadINTELacc_32x64_f32_p1i8_i64_i32_local(i8 addrspace(3)*, i64, i32)
declare void @__builtin_spirv_OpJointMatrixStoreINTELacc_32x64_f32_p1i8_acc_32x64_f32_i64_i32_generic(i8*, %intel.joint_matrix_acc_32x64_f32_t *, i64, i32)
declare void @__builtin_spirv_OpJointMatrixStoreINTELacc_32x64_f32_p1i8_acc_32x64_f32_i64_i32_global(i8 addrspace(1)*, %intel.joint_matrix_acc_32x64_f32_t *, i64, i32)
declare void @__builtin_spirv_OpJointMatrixStoreINTELacc_32x64_f32_p1i8_acc_32x64_f32_i64_i32_local(i8 addrspace(3)*, %intel.joint_matrix_acc_32x64_f32_t *, i64, i32)

!igc.functions = !{!0, !4, !5}
!0 = !{void (i8*, i8*)* @test_generic, !1}
!4 = !{void (i8 addrspace(1)*, i8 addrspace(1)*)* @test_global, !1}
!5 = !{void (i8 addrspace(3)*, i8 addrspace(3)*)* @test_local, !1}
!1 = !{!2, !3}
!2 = !{!"function_type", i32 0}
!3 = !{!"sub_group_size", i32 8}
