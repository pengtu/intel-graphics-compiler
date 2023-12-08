;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2021 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; Check that bti is assigned to i32 state arguments and
; bindless buffers are still passed as kernel arguments.

; RUN: %opt %use_old_pass_manager% -GenXBTIAssignment -vc-use-bindless-buffers -march=genx64 -mcpu=Gen9 -S < %s | FileCheck %s

target datalayout = "e-p:64:64-i64:64-n8:16:32:64"
target triple = "spir64-unknown-unknown"

declare void @use_value(i32)
declare void @use_value64(i64)

; CHECK-LABEL: @simple(
define dllexport spir_kernel void @simple(i32 %surf, i32 %samp) #0 {
; CHECK:      call void @use_value(i32 %surf)
; CHECK-NEXT: call void @use_value(i32 0)
  call void @use_value(i32 %surf)
  call void @use_value(i32 %samp)
  ret void
}

; CHECK-LABEL: @mixed_srv_uav(
define dllexport spir_kernel void @mixed_srv_uav(i32 %image_ro, i32 %image_rw, i32 %buf, i32 %image_ro2) #0 {
; CHECK:      call void @use_value(i32 0)
; CHECK-NEXT: call void @use_value(i32 2)
; CHECK-NEXT: call void @use_value(i32 %buf)
; CHECK-NEXT: call void @use_value(i32 1)
  call void @use_value(i32 %image_ro)
  call void @use_value(i32 %image_rw)
  call void @use_value(i32 %buf)
  call void @use_value(i32 %image_ro2)
  ret void
}

; CHECK-LABEL: @mixed_all(
define dllexport spir_kernel void @mixed_all(i32 %imwo, i32 %imro, i32 %plain, i32 %samp1, i64 %svm, i32 %buf, i32 %samp2) #0 {
; CHECK:      call void @use_value(i32 1)
; CHECK-NEXT: call void @use_value(i32 0)
; CHECK-NEXT: call void @use_value(i32 %plain)
; CHECK-NEXT: call void @use_value(i32 0)
; CHECK-NEXT: call void @use_value64(i64 %svm)
; CHECK-NEXT: call void @use_value(i32 %buf)
; CHECK-NEXT: call void @use_value(i32 1)
  call void @use_value(i32 %imwo)
  call void @use_value(i32 %imro)
  call void @use_value(i32 %plain)
  call void @use_value(i32 %samp1)
  call void @use_value64(i64 %svm)
  call void @use_value(i32 %buf)
  call void @use_value(i32 %samp2)
  ret void
}

attributes #0 = { "CMGenxMain" }

!genx.kernels = !{!0, !5, !10}
!genx.kernel.internal = !{!4, !9, !14}
; CHECK: !genx.kernel.internal = !{[[SIMPLE_NODE:![0-9]+]], [[MIXED_NODE:![0-9]+]], [[MIXED_ALL_NODE:![0-9]+]]}
; CHECK-DAG: [[SIMPLE_NODE]] = !{void (i32, i32)* @simple, null, null, null, [[SIMPLE_BTIS:![0-9]+]]}
; CHECK-DAG: [[SIMPLE_BTIS]] = !{i32 0, i32 0}
; CHECK-DAG: [[MIXED_NODE]] = !{void (i32, i32, i32, i32)* @mixed_srv_uav, null, null, null, [[MIXED_BTIS:![0-9]+]]}
; CHECK-DAG: [[MIXED_BTIS]] = !{i32 0, i32 2, i32 3, i32 1}
; CHECK-DAG: [[MIXED_ALL_NODE]] = !{void (i32, i32, i32, i32, i64, i32, i32)* @mixed_all, null, null, null, [[MIXED_ALL_BTIS:![0-9]+]]}
; CHECK-DAG: [[MIXED_ALL_BTIS]] = !{i32 1, i32 0, i32 -1, i32 0, i32 255, i32 2, i32 1}

!0 = !{void (i32, i32)* @simple, !"simple", !1, i32 0, i32 0, !2, !3, i32 0}
!1 = !{i32 2, i32 1}
!2 = !{i32 0, i32 0}
!3 = !{!"buffer_t read_write", !"sampler_t"}
!4 = !{void (i32, i32)* @simple, null, null, null, null}

!5 = !{void (i32, i32, i32, i32)* @mixed_srv_uav, !"mixed_srv_uav", !6, i32 0, i32 0, !7, !8, i32 0}
!6 = !{i32 2, i32 2, i32 2, i32 2}
!7 = !{i32 0, i32 0, i32 0, i32 0}
!8 = !{!"image2d_t read_only", !"image2d_t read_write", !"buffer_t", !"image2d_t read_only"}
!9 = !{void (i32, i32, i32, i32)* @mixed_srv_uav, null, null, null, null}

!10 = !{void (i32, i32, i32, i32, i64, i32, i32)* @mixed_all, !"mixed_srv_uav", !11, i32 0, i32 0, !12, !13, i32 0}
!11 = !{i32 2, i32 2, i32 0, i32 1, i32 0, i32 2, i32 1}
!12 = !{i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0}
!13 = !{!"image2d_t write_only", !"image2d_t read_only", !"", !"sampler_t", !"svmptr_t", !"buffer_t", !"sampler_t"}
!14 = !{void (i32, i32, i32, i32, i64, i32, i32)* @mixed_all, null, null, null, null}
