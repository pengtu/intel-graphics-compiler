;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2022 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================
;
; RUN: %opt %use_old_pass_manager% -GenXAggregatePseudoLowering -march=genx64 -mtriple=spir64-unknown-unknown  -mcpu=Gen9 -S < %s | FileCheck %s
; ------------------------------------------------
; GenXAggregatePseudoLowering
; ------------------------------------------------
; This test checks that GenXAggregatePseudoLowering pass follows
; 'How to Update Debug Info' llvm guideline.
;
; Debug MD for this test was created with debugify pass.

; CHECK: void @test_aggrlower{{.*}} !dbg [[SCOPE:![0-9]*]]
; CHECK: entry:
; CHECK: {{%[A-z0-9.]*}} = {{.*}}, !dbg [[VAL1_LOC:![0-9]*]]
; CHECK: void @llvm.dbg.value(metadata %struct.st {{%[A-z0-9.]*}}, metadata [[VAL1_MD:![0-9]*]], metadata !DIExpression()), !dbg [[VAL1_LOC]]
; CHECK: bb1:
; CHECK: {{%[A-z0-9.]*}} = {{.*}}, !dbg [[VAL2_LOC:![0-9]*]]
; CHECK: void @llvm.dbg.value(metadata %struct.st {{%[A-z0-9.]*}}, metadata [[VAL2_MD:![0-9]*]], metadata !DIExpression()), !dbg [[VAL2_LOC]]
; CHECK: {{%[A-z0-9.]*}} = {{.*}}, !dbg [[VAL3_LOC:![0-9]*]]
; CHECK: void @llvm.dbg.value(metadata %struct.st {{%[A-z0-9.]*}}, metadata [[VAL3_MD:![0-9]*]], metadata !DIExpression()), !dbg [[VAL3_LOC]]
; CHECK: {{%[A-z0-9.]*}} = {{.*}}, !dbg [[VAL4_LOC:![0-9]*]]
; CHECK: void @llvm.dbg.value(metadata i1 {{%[A-z0-9.]*}}, metadata [[VAL4_MD:![0-9]*]], metadata !DIExpression()), !dbg [[VAL4_LOC]]
; CHECK: {{%[A-z0-9.]*}} = {{.*}}, !dbg [[VAL5_LOC:![0-9]*]]
; CHECK: void @llvm.dbg.value(metadata %struct.st {{%[A-z0-9.]*}}, metadata [[VAL5_MD:![0-9]*]], metadata !DIExpression()), !dbg [[VAL5_LOC]]
; CHECK: end:
; CHECK: store{{.*}}, !dbg [[STR1_LOC:![0-9]*]]

%struct.st = type { i1, <4 x float>, <2 x i64> }

define void @test_aggrlower(%struct.st* %src, %struct.st* %dst) !dbg !6 {
entry:
  %0 = load %struct.st, %struct.st* %src, !dbg !16
  call void @llvm.dbg.value(metadata %struct.st %0, metadata !9, metadata !DIExpression()), !dbg !16
  br label %bb1, !dbg !17

bb1:                                              ; preds = %bb1, %entry
  %1 = phi %struct.st [ %0, %entry ], [ %2, %bb1 ], !dbg !18
  call void @llvm.dbg.value(metadata %struct.st %1, metadata !11, metadata !DIExpression()), !dbg !18
  %2 = load %struct.st, %struct.st* %dst, !dbg !19
  call void @llvm.dbg.value(metadata %struct.st %2, metadata !12, metadata !DIExpression()), !dbg !19
  %3 = extractvalue %struct.st %1, 0, !dbg !20
  call void @llvm.dbg.value(metadata i1 %3, metadata !13, metadata !DIExpression()), !dbg !20
  %4 = select i1 %3, %struct.st %2, %struct.st %0, !dbg !21
  call void @llvm.dbg.value(metadata %struct.st %4, metadata !15, metadata !DIExpression()), !dbg !21
  br i1 %3, label %end, label %bb1, !dbg !22

end:                                              ; preds = %bb1
  store %struct.st %4, %struct.st* %dst, !dbg !23
  ret void, !dbg !24
}

; CHECK-DAG: [[FILE:![0-9]*]] = !DIFile(filename: "struct.ll", directory: "/")
; CHECK-DAG: [[SCOPE]] = distinct !DISubprogram(name: "test_aggrlower", linkageName: "test_aggrlower", scope: null, file: [[FILE]], line: 1
; CHECK-DAG: [[VAL1_MD]] = !DILocalVariable(name: "1", scope: [[SCOPE]], file: [[FILE]], line: 1
; CHECK-DAG: [[VAL1_LOC]] = !DILocation(line: 1, column: 1, scope: [[SCOPE]])
; CHECK-DAG: [[VAL2_MD]] = !DILocalVariable(name: "2", scope: [[SCOPE]], file: [[FILE]], line: 3
; CHECK-DAG: [[VAL2_LOC]] = !DILocation(line: 3, column: 1, scope: [[SCOPE]])
; CHECK-DAG: [[VAL3_MD]] = !DILocalVariable(name: "3", scope: [[SCOPE]], file: [[FILE]], line: 4
; CHECK-DAG: [[VAL3_LOC]] = !DILocation(line: 4, column: 1, scope: [[SCOPE]])
; CHECK-DAG: [[VAL4_MD]] = !DILocalVariable(name: "4", scope: [[SCOPE]], file: [[FILE]], line: 5
; CHECK-DAG: [[VAL4_LOC]] = !DILocation(line: 5, column: 1, scope: [[SCOPE]])
; CHECK-DAG: [[VAL5_MD]] = !DILocalVariable(name: "5", scope: [[SCOPE]], file: [[FILE]], line: 6
; CHECK-DAG: [[VAL5_LOC]] = !DILocation(line: 6, column: 1, scope: [[SCOPE]])
; CHECK-DAG: [[STR1_LOC]] = !DILocation(line: 8, column: 1, scope: [[SCOPE]])

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.value(metadata, metadata, metadata) #0

attributes #0 = { nounwind readnone speculatable }

!llvm.dbg.cu = !{!0}
!llvm.debugify = !{!3, !4}
!llvm.module.flags = !{!5}

!0 = distinct !DICompileUnit(language: DW_LANG_C, file: !1, producer: "debugify", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "struct.ll", directory: "/")
!2 = !{}
!3 = !{i32 9}
!4 = !{i32 5}
!5 = !{i32 2, !"Debug Info Version", i32 3}
!6 = distinct !DISubprogram(name: "test_aggrlower", linkageName: "test_aggrlower", scope: null, file: !1, line: 1, type: !7, scopeLine: 1, unit: !0, retainedNodes: !8)
!7 = !DISubroutineType(types: !2)
!8 = !{!9, !11, !12, !13, !15}
!9 = !DILocalVariable(name: "1", scope: !6, file: !1, line: 1, type: !10)
!10 = !DIBasicType(name: "ty384", size: 384, encoding: DW_ATE_unsigned)
!11 = !DILocalVariable(name: "2", scope: !6, file: !1, line: 3, type: !10)
!12 = !DILocalVariable(name: "3", scope: !6, file: !1, line: 4, type: !10)
!13 = !DILocalVariable(name: "4", scope: !6, file: !1, line: 5, type: !14)
!14 = !DIBasicType(name: "ty8", size: 8, encoding: DW_ATE_unsigned)
!15 = !DILocalVariable(name: "5", scope: !6, file: !1, line: 6, type: !10)
!16 = !DILocation(line: 1, column: 1, scope: !6)
!17 = !DILocation(line: 2, column: 1, scope: !6)
!18 = !DILocation(line: 3, column: 1, scope: !6)
!19 = !DILocation(line: 4, column: 1, scope: !6)
!20 = !DILocation(line: 5, column: 1, scope: !6)
!21 = !DILocation(line: 6, column: 1, scope: !6)
!22 = !DILocation(line: 7, column: 1, scope: !6)
!23 = !DILocation(line: 8, column: 1, scope: !6)
!24 = !DILocation(line: 9, column: 1, scope: !6)
