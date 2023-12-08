;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2023 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================
;
;-----------------------------------------------------------------------------------------
; Standalone mode
;-----------------------------------------------------------------------------------------
; RUN: %opt %use_old_pass_manager% -GenXGVClobberChecker -check-gv-clobbering-standalone-mode=true -check-gv-clobbering=true -check-gv-clobbering-collect-kill-call-sites=true -check-gv-clobbering-try-fixup=true -check-gv-clobbering-abort-on-detection=false -march=genx64 -mtriple=spir64-unknown-unknown -mcpu=Gen9 -S < %s | FileCheck %s
;-----------------------------------------------------------------------------------------
; RUN: %opt %use_old_pass_manager% -GenXGVClobberChecker -check-gv-clobbering-standalone-mode=true -check-gv-clobbering=true -check-gv-clobbering-collect-kill-call-sites=false -check-gv-clobbering-try-fixup=true -check-gv-clobbering-abort-on-detection=false -march=genx64 -mtriple=spir64-unknown-unknown -mcpu=Gen9 -S < %s | FileCheck %s
;-----------------------------------------------------------------------------------------
; In-pipeline mode (simulating situation when running during normal compilation)
;-----------------------------------------------------------------------------------------
; RUN: %opt %use_old_pass_manager% -GenXModule -GenXCategoryWrapper -GenXGVClobberChecker -check-gv-clobbering-standalone-mode=false -check-gv-clobbering=true -check-gv-clobbering-collect-kill-call-sites=true -check-gv-clobbering-try-fixup=true -check-gv-clobbering-abort-on-detection=false -march=genx64 -mtriple=spir64-unknown-unknown -mcpu=Gen9 -S < %s | FileCheck %s
; ------------------------------------------------
; This test checks global volatile clobbering checker/fixup introduced late in pipeline to catch over-optimizations of global volatile access. This is an auxiliary utility used to help in detecting and fixing erroneous over-optimizations cases. The checker/fixup is only available under the -check-gv-clobbering=true option and for a limited number of cases.

target datalayout = "e-p:64:64-p6:32:32-i64:64-n8:16:32:64"
target triple = "genx64-unknown-unknown"

@_ZL8g_global = external global <4 x i32> #0

define spir_kernel void @TestGVClobberingFixupStoreInCall(i8 addrspace(1)* %_arg_buf_gpu, i8 addrspace(1)* %_arg_res_gpu) #1 {
entry:
  %call.i.i.i8.i.esimd6 = load volatile <4 x i32>, <4 x i32>* @_ZL8g_global, align 16
  tail call spir_func void @UserFunctionRewriteGV1()
  %vecext.i.i1.regioncollapsed = tail call i32 @llvm.genx.rdregioni.i32.v4i32.i16(<4 x i32> %call.i.i.i8.i.esimd6, i32 0, i32 1, i32 1, i16 0, i32 undef)
  %cmp.i.i = icmp eq i32 %vecext.i.i1.regioncollapsed, 55
  %conv.i.i4 = select i1 %cmp.i.i, i32 0, i32 0
  %bitcast = bitcast i32 %conv.i.i4 to <1 x i32>
  call void @llvm.genx.svm.scatter.v1i1.v1i64.v1i32(<1 x i1> zeroinitializer, i32 0, <1 x i64> zeroinitializer, <1 x i32> %bitcast)
  ret void
}

define spir_kernel void @TestGVClobberingFixupLocalStore(i8 addrspace(1)* %_arg_buf_gpu, i8 addrspace(1)* %_arg_res_gpu, i64 %impl.arg.private.base) #1 {
entry:
  %call.i.i.i8.i.esimd6 = load volatile <4 x i32>, <4 x i32>* @_ZL8g_global, align 16
  %call4.i.i.i.i.i.i.esimd = tail call <4 x i32> @llvm.genx.wrregioni.v4i32.v1i32.i16.v1i1(<4 x i32> %call.i.i.i8.i.esimd6, <1 x i32> <i32 42>, i32 0, i32 1, i32 1, i16 0, i32 0, <1 x i1> <i1 true>)
  store volatile <4 x i32> %call4.i.i.i.i.i.i.esimd, <4 x i32>* @_ZL8g_global, align 16
  %vecext.i.i1.regioncollapsed = tail call i32 @llvm.genx.rdregioni.i32.v4i32.i16(<4 x i32> %call.i.i.i8.i.esimd6, i32 0, i32 1, i32 1, i16 0, i32 undef)
  %cmp.i.i = icmp eq i32 %vecext.i.i1.regioncollapsed, 55
  %conv.i.i4 = select i1 %cmp.i.i, i32 0, i32 0
  %bitcast = bitcast i32 %conv.i.i4 to <1 x i32>
  call void @llvm.genx.svm.scatter.v1i1.v1i64.v1i32(<1 x i1> zeroinitializer, i32 0, <1 x i64> zeroinitializer, <1 x i32> %bitcast)
  ret void
}

define spir_kernel void @TestGVClobberingFixupLoopLocalStore(i8 addrspace(1)* %_arg_input_gpu, i8 addrspace(1)* %_arg_res_gpu, i64 %impl.arg.private.base) #1 {
entry:
  %bitcast = bitcast <1 x i32> zeroinitializer to i32
  %call.i.i.i17.i.esimd8 = load volatile <4 x i32>, <4 x i32>* @_ZL8g_global, align 16
  %cmp.i1 = icmp sgt i32 %bitcast, 0
  br i1 %cmp.i1, label %entry.while.body.i_crit_edge, label %entry.TestGVClobberingFixupLoopLocalStore.exit_crit_edge

entry.TestGVClobberingFixupLoopLocalStore.exit_crit_edge: ; preds = %entry
  br label %entry.while.body.i_crit_edge

entry.while.body.i_crit_edge:                     ; preds = %entry.TestGVClobberingFixupLoopLocalStore.exit_crit_edge, %entry
  %.splatinsert35 = bitcast i64 0 to <1 x i64>
  br label %while.body.i

while.body.i:                                     ; preds = %while.body.i.while.body.i_crit_edge, %entry.while.body.i_crit_edge
  %p2.0.i2 = phi i32 [ %dec.i, %while.body.i.while.body.i_crit_edge ], [ 0, %entry.while.body.i_crit_edge ]
  %dec.i = add nsw i32 %p2.0.i2, -1
  %vecext.i.i3.regioncollapsed = call <1 x i32> @llvm.genx.rdregioni.v1i32.v4i32.i16(<4 x i32> %call.i.i.i17.i.esimd8, i32 0, i32 1, i32 1, i16 0, i32 undef)
  call void @llvm.genx.svm.scatter.v1i1.v1i64.v1i32(<1 x i1> <i1 true>, i32 0, <1 x i64> %.splatinsert35, <1 x i32> %vecext.i.i3.regioncollapsed)
  store volatile <4 x i32> zeroinitializer, <4 x i32>* @_ZL8g_global, align 16
  br label %while.body.i.while.body.i_crit_edge

while.body.i.while.body.i_crit_edge:              ; preds = %while.body.i
  br label %while.body.i
}

define internal spir_func void @UserFunctionRewriteGV1() {
entry:
  tail call spir_func void @UserFunctionRewriteGV2()
  ret void
}

define internal spir_func void @UserFunctionRewriteGV2() {
entry:
  store volatile <4 x i32> zeroinitializer, <4 x i32>* @_ZL8g_global, align 16
  ret void
}

declare i32 @llvm.genx.rdregioni.i32.v4i32.i16(<4 x i32>, i32, i32, i32, i16, i32)
declare void @llvm.genx.svm.scatter.v1i1.v1i64.v1i32(<1 x i1>, i32, <1 x i64>, <1 x i32>)
declare <1 x i32> @llvm.genx.rdregioni.v1i32.v4i32.i16(<4 x i32>, i32, i32, i32, i16, i32)
declare <4 x i32> @llvm.genx.wrregioni.v4i32.v1i32.i16.v1i1(<4 x i32>, <1 x i32>, i32, i32, i32, i16, i32, <1 x i1>)

attributes #0 = { "genx_volatile" }
attributes #1 = { "CMGenxMain" }

!genx.kernels = !{!0, !5, !6}
!genx.kernel.internal = !{!7, !12, !13}

!0 = !{void (i8 addrspace(1)*, i8 addrspace(1)*)* @TestGVClobberingFixupStoreInCall, !"TestGVClobberingFixupStoreInCall", !1, i32 0, !2, !3, !4, i32 0}
!1 = !{i32 0, i32 0, i32 96}
!2 = !{i32 72, i32 80, i32 64}
!3 = !{i32 0, i32 0}
!4 = !{!"svmptr_t", !"svmptr_t"}
!5 = !{void (i8 addrspace(1)*, i8 addrspace(1)*, i64)* @TestGVClobberingFixupLocalStore, !"TestGVClobberingFixupLocalStore", !1, i32 0, !2, !3, !4, i32 0}
!6 = !{void (i8 addrspace(1)*, i8 addrspace(1)*, i64)* @TestGVClobberingFixupLoopLocalStore, !"TestGVClobberingFixupLoopLocalStore", !1, i32 0, !2, !3, !4, i32 0}
!7 = !{void (i8 addrspace(1)*, i8 addrspace(1)*)* @TestGVClobberingFixupStoreInCall, !8, !9, !10, !11}
!8 = !{i32 0, i32 0, i32 0}
!9 = !{i32 0, i32 1, i32 2}
!10 = !{}
!11 = !{i32 255, i32 255, i32 255}
!12 = !{void (i8 addrspace(1)*, i8 addrspace(1)*, i64)* @TestGVClobberingFixupLocalStore, !8, !9, !10, !11}
!13 = !{void (i8 addrspace(1)*, i8 addrspace(1)*, i64)* @TestGVClobberingFixupLoopLocalStore, !8, !9, !10, !11}

;-------------------------------------------------------------------------------------------
; CHECK:  %call.i.i.i8.i.esimd6 = load volatile <4 x i32>, <4 x i32>* @_ZL8g_global
; CHECK-NEXT:  %vecext.i.i1.regioncollapsed = tail call i32 @llvm.genx.rdregioni.i32.v4i32.i16(<4 x i32> %call.i.i.i8.i.esimd6, i32 0, i32 1, i32 1, i16 0, i32 undef)
; COM: if -check-gv-clobbering-collect-kill-call-sites=true is supplied
; COM: store interference is precisely detected here down the call chain.
; COM: if -check-gv-clobbering-collect-kill-call-sites=false is supplied or omitted
; COM: store interference is speculated because of call to a user function.
; CHECK-NEXT:  tail call spir_func void @UserFunctionRewriteGV1()
; CHECK-NOT:  %vecext.i.i1.regioncollapsed = tail call i32 @llvm.genx.rdregioni.i32.v4i32.i16(<4 x i32> %call.i.i.i8.i.esimd6, i32 0, i32 1, i32 1, i16 0, i32 undef)
; CHECK-NEXT:  %cmp.i.i = icmp eq i32 %vecext.i.i1.regioncollapsed, 55

;-------------------------------------------------------------------------------------------
; CHECK:  %call.i.i.i8.i.esimd6 = load volatile <4 x i32>, <4 x i32>* @_ZL8g_global
; CHECK-NEXT:  %vecext.i.i1.regioncollapsed = tail call i32 @llvm.genx.rdregioni.i32.v4i32.i16(<4 x i32> %call.i.i.i8.i.esimd6, i32 0, i32 1, i32 1, i16 0, i32 undef)
; COM: store interference is directly detected in this function.
; CHECK-NEXT:  %call4.i.i.i.i.i.i.esimd = tail call <4 x i32> @llvm.genx.wrregioni.v4i32.v1i32.i16.v1i1(<4 x i32> %call.i.i.i8.i.esimd6, <1 x i32> <i32 42>, i32 0, i32 1, i32 1, i16 0, i32 0, <1 x i1> <i1 true>)
; CHECK-NEXT: store volatile <4 x i32> %call4.i.i.i.i.i.i.esimd, <4 x i32>* @_ZL8g_global
; CHECK-NOT:  %vecext.i.i1.regioncollapsed = tail call i32 @llvm.genx.rdregioni.i32.v4i32.i16(<4 x i32> %call.i.i.i8.i.esimd6, i32 0, i32 1, i32 1, i16 0, i32 undef)
; CHECK-NEXT:  %cmp.i.i = icmp eq i32 %vecext.i.i1.regioncollapsed, 55

;-------------------------------------------------------------------------------------------
; CHECK:  %call.i.i.i17.i.esimd8 = load volatile <4 x i32>, <4 x i32>* @_ZL8g_global
; CHECK-NEXT:  %vecext.i.i3.regioncollapsed = call <1 x i32> @llvm.genx.rdregioni.v1i32.v4i32.i16(<4 x i32> %call.i.i.i17.i.esimd8, i32 0, i32 1, i32 1, i16 0, i32 undef)
; CHECK-NEXT:  %cmp.i1 = icmp sgt i32 %bitcast, 0

;-------------------------------------------------------------------------------------------
; CHECK: %dec.i = add nsw i32 %p2.0.i2, -1
; CHECK-NOT:  %vecext.i.i3.regioncollapsed = call <1 x i32> @llvm.genx.rdregioni.v1i32.v4i32.i16(<4 x i32> %call.i.i.i17.i.esimd8, i32 0, i32 1, i32 1, i16 0, i32 undef)
; CHECK-NEXT: call void @llvm.genx.svm.scatter.v1i1.v1i64.v1i32(<1 x i1> <i1 true>, i32 0, <1 x i64> %.splatinsert35, <1 x i32> %vecext.i.i3.regioncollapsed)
