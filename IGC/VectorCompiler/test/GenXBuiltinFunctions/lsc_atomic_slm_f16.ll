;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2023 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; RUN: %opt %use_old_pass_manager% -GenXBuiltinFunctions -march=genx64 -mtriple=spir64-unknown-unknown \
; RUN: -mcpu=XeLPG -S < %s 2>&1 | FileCheck %s
; RUN: %opt %use_old_pass_manager% -GenXBuiltinFunctions -march=genx64 -mtriple=spir64-unknown-unknown \
; RUN: -mcpu=XeHPC -S < %s 2>&1 | FileCheck --check-prefix=CHECK-2 %s

; CHECK-NOT: WARNING
; CHECK-2-NOT: WARNING

; CHECK: @test_fadd_kernel
; CHECK: = call <16 x i32> @__vc_builtin_atomic_slm_v16i32_v2i8
; CHECK: ret void
; CHECK: @test_fcas_kernel
; CHECK: = tail call <16 x i32> @llvm.vc.internal.lsc.atomic.slm.v16i32.v16i1.v2i8.v16i32
; CHECK: ret void
; CHECK: @test_store_kernel
; CHECK: = tail call <16 x i32> @llvm.vc.internal.lsc.atomic.slm.v16i32.v16i1.v2i8.v16i32
; CHECK: ret void
; CHECK: @test_load_kernel
; CHECK: = tail call <16 x i32> @llvm.vc.internal.lsc.atomic.slm.v16i32.v16i1.v2i8.v16i32
; CHECK: ret void
; CHECK-2: @test_fadd_kernel
; CHECK-2: = call <16 x i32> @__vc_builtin_atomic_slm_v16i32_v2i8
; CHECK-2: ret void
; CHECK-2: @test_fcas_kernel
; CHECK-2: = tail call <16 x i32> @llvm.vc.internal.lsc.atomic.slm.v16i32.v16i1.v2i8.v16i32
; CHECK-2: ret void
; CHECK-2: @test_store_kernel
; CHECK-2: = tail call <16 x i32> @llvm.vc.internal.lsc.atomic.slm.v16i32.v16i1.v2i8.v16i32
; CHECK-2: ret void
; CHECK-2: @test_load_kernel
; CHECK-2: = tail call <16 x i32> @llvm.vc.internal.lsc.atomic.slm.v16i32.v16i1.v2i8.v16i32
; CHECK-2: ret void

declare <16 x i32> @llvm.vc.internal.lsc.atomic.slm.v16i32.v16i1.v2i8.v16i32(<16 x i1>, i8, i8, i8, <2 x i8>, i32, <16 x i32>, i16, i32, <16 x i32>, <16 x i32>, <16 x i32>)

define dllexport spir_kernel void @test_fadd_kernel(<16 x i1> %pred, <16 x i32> %index, <16 x i32> %src1, <16 x i32> %src2, <16 x i32> %passthru) {
  %1 = tail call <16 x i32> @llvm.vc.internal.lsc.atomic.slm.v16i32.v16i1.v2i8.v16i32(<16 x i1> %pred, i8 19, i8 2, i8 6, <2 x i8> zeroinitializer, i32 0, <16 x i32> %index, i16 1, i32 0, <16 x i32> %src1, <16 x i32> %src2, <16 x i32> %passthru)
  ret void
}

define dllexport spir_kernel void @test_fcas_kernel(<16 x i1> %pred, <16 x i32> %index, <16 x i32> %src1, <16 x i32> %src2, <16 x i32> %passthru) {
  %1 = tail call <16 x i32> @llvm.vc.internal.lsc.atomic.slm.v16i32.v16i1.v2i8.v16i32(<16 x i1> %pred, i8 23, i8 2, i8 6, <2 x i8> zeroinitializer, i32 0, <16 x i32> %index, i16 1, i32 0, <16 x i32> %src1, <16 x i32> %src2, <16 x i32> %passthru)
  ret void
}

define dllexport spir_kernel void @test_store_kernel(<16 x i1> %pred, <16 x i32> %index, <16 x i32> %src1, <16 x i32> %src2, <16 x i32> %passthru) {
  %1 = tail call <16 x i32> @llvm.vc.internal.lsc.atomic.slm.v16i32.v16i1.v2i8.v16i32(<16 x i1> %pred, i8 11, i8 2, i8 6, <2 x i8> zeroinitializer, i32 0, <16 x i32> %index, i16 1, i32 0, <16 x i32> %src1, <16 x i32> %src2, <16 x i32> %passthru)
  ret void
}

define dllexport spir_kernel void @test_load_kernel(<16 x i1> %pred, <16 x i32> %index, <16 x i32> %src1, <16 x i32> %src2, <16 x i32> %passthru) {
  %1 = tail call <16 x i32> @llvm.vc.internal.lsc.atomic.slm.v16i32.v16i1.v2i8.v16i32(<16 x i1> %pred, i8 10, i8 2, i8 6, <2 x i8> zeroinitializer, i32 0, <16 x i32> %index, i16 1, i32 0, <16 x i32> %src1, <16 x i32> %src2, <16 x i32> %passthru)
  ret void
}

; COM: The presence of these __vc_builtin_* funcitions is a HACK to trick VC
; COM: backend into thinking that we have built-in routines
define <16 x i32> @__vc_builtin_atomic_slm_v16i32_v2i8(<16 x i8> noundef %pred, i8 noundef signext %op, <2 x i8> %cachecontrols, i32 noundef %base, <16 x i32> noundef %index, i16 noundef signext %scale, i32 noundef %offset, <16 x i32> noundef %src1, <16 x i32> noundef %src2, <16 x i32> noundef %passthru) #0 {
  ret <16 x i32> zeroinitializer
}

attributes #0 = { "VC.Builtin" }
