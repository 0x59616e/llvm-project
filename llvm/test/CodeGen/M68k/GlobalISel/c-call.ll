; RUN: llc -mtriple=m68k -global-isel -verify-machineinstrs %s -o - | FileCheck %s

define i32 @test1() nounwind {
; CHECK-LABEL: test1:
; CHECK:       ; %bb.0
; CHECK-NEXT:  suba.l  #4, %sp
; CHECK-NEXT:  jsr     callee1
; CHECK-NEXT:  move.l  #0, %d0
; CHECK-NEXT:  adda.l  #4, %sp
; CHECK-NEXT:  rts
  call void @callee1() nounwind
  ret i32 0
}
declare void @callee1();

define i32 @test2() nounwind {
; CHECK-LABEL: test2:
; CHECK:       ; %bb.0:
; CHECK-NEXT:  suba.l  #12, %sp
; CHECK-NEXT:  move.l  #12, (0,%sp)
; CHECK-NEXT:  move.l  #18, (4,%sp)
; CHECK-NEXT:  move.l  #-4, (8,%sp)
; CHECK-NEXT:  jsr     callee2
; CHECK-NEXT:  move.l  #0, %d0
; CHECK-NEXT:  adda.l  #12, %sp
; CHECK-NEXT:  rts
  call void @callee2(i32 12, i32 18, i32 -4) nounwind
  ret i32 0
}
declare void @callee2(i32, i32, i32);

define i32 @test3(i32 %0, i32 %1) nounwind {
; CHECK-LABEL: test3:
; CHECK:       ; %bb.0:
; CHECK-NEXT:  suba.l  #12, %sp
; CHECK-NEXT:  move.l  (16,%sp), %d0
; CHECK-NEXT:  move.l  (20,%sp), %d1
; CHECK-NEXT:  move.l  %d0, (0,%sp)
; CHECK-NEXT:  move.l  %d1, (4,%sp)
; CHECK-NEXT:  jsr     callee3
; CHECK-NEXT:  adda.l  #12, %sp
; CHECK-NEXT:  rts
  %3 = call i32 @callee3(i32 %0, i32 %1)
  ret i32 %3
}
<<<<<<< HEAD
declare i32 @callee3(i32, i32)

define i64 @test_ret_i64(i64 %0) nounwind {
; CHECK-LABEL: test_ret_i64:
; CHECK:       ; %bb.0
; CHECK-NEXT:  move.l (8,%sp), %d1
; CHECK-NEXT:  move.l (4,%sp), %d0
; CHECK-NEXT:  rts
  ret i64 %0
}

define void @test_passing_i64(i64 %0, i64 %1) nounwind {
; CHECK-LABEL: test_passing_i64:
; CHECK:       ; %bb.0:
; CHECK-NEXT:  suba.l  #20, %sp
; CHECK-NEXT:  move.l (28,%sp), %d0
; CHECK-NEXT:  move.l (24,%sp), %d1
; CHECK-NEXT:  move.l (36,%sp), %a0
; CHECK-NEXT:  move.l (32,%sp), %a1
; CHECK-NEXT:  move.l %a0, (4,%sp)
; CHECK-NEXT:  move.l %a1, (0,%sp)
; CHECK-NEXT:  move.l %d0, (12,%sp)
; CHECK-NEXT:  move.l %d1, (8,%sp)
; CHECK-NEXT:  jsr callee_test_passing_i64
; CHECK-NEXT:  adda.l #20, %sp
; CHECK-NEXT:  rts
  call void @callee_test_passing_i64(i64 %1, i64 %0)
  ret void
}
declare void @callee_test_passing_i64(i64, i64)
=======
declare i32 @callee3(i32, i32)
>>>>>>> d95c2eb3e699... [M68k][InstructionSelector]
