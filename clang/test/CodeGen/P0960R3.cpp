// RUN: %clang_cc1 -std=c++20 %s -emit-llvm -o - | FileCheck %s

struct A {
  char i;
  double j;
};

struct B {
  A a;
  int b;
};

struct C : public B, public A {};

// CHECK-LABEL:   entry:
// CHECK-NEXT:    [[A1:%.*]] = alloca [[STRUCT_A:%.*]], align 8
// CHECK-NEXT:    [[B1:%.*]] = alloca [[STRUCT_B:%.*]], align 8
// CHECK-NEXT:    [[C1:%.*]] = alloca [[STRUCT_C:%.*]], align 8
// CHECK-NEXT:    [[REF_TMP:%.*]] = alloca [[STRUCT_B]], align 8
// CHECK-NEXT:    [[REF_TMP5:%.*]] = alloca [[STRUCT_A]], align 8
// CHECK-NEXT:    [[I:%.*]] = getelementptr inbounds [[STRUCT_A]], ptr [[A1]], i32 0, i32 0
// CHECK-NEXT:    store i8 3, ptr [[I]], align 8
// CHECK-NEXT:    [[J:%.*]] = getelementptr inbounds [[STRUCT_A]], ptr [[A1]], i32 0, i32 1
// CHECK-NEXT:    store double 2.000000e+00, ptr [[J]], align 8
// CHECK-NEXT:    [[A:%.*]] = getelementptr inbounds [[STRUCT_B]], ptr [[B1]], i32 0, i32 0
// CHECK-NEXT:    [[I1:%.*]] = getelementptr inbounds [[STRUCT_A]], ptr [[A]], i32 0, i32 0
// CHECK-NEXT:    store i8 99, ptr [[I1]], align 8
// CHECK-NEXT:    [[A2:%.*]] = getelementptr inbounds [[STRUCT_B]], ptr [[REF_TMP]], i32 0, i32 0
// CHECK-NEXT:    [[I3:%.*]] = getelementptr inbounds [[STRUCT_A]], ptr [[A2]], i32 0, i32 0
// CHECK-NEXT:    store i8 1, ptr [[I3]], align 8
// CHECK-NEXT:    [[J4:%.*]] = getelementptr inbounds [[STRUCT_A]], ptr [[A2]], i32 0, i32 1
// CHECK-NEXT:    store double 1.000000e+00, ptr [[J4]], align 8
// CHECK-NEXT:    [[B:%.*]] = getelementptr inbounds [[STRUCT_B]], ptr [[REF_TMP]], i32 0, i32 1
// CHECK-NEXT:    store i32 1, ptr [[B]], align 8
// CHECK-NEXT:    call void @llvm.memcpy.p0.p0.i64(ptr align 8 [[C1]], ptr align 8 [[REF_TMP]], i64 24, i1 false)
// CHECK-NEXT:    [[TMP0:%.*]] = getelementptr inbounds i8, ptr [[C1]], i64 24
// CHECK-NEXT:    [[I6:%.*]] = getelementptr inbounds [[STRUCT_A]], ptr [[REF_TMP5]], i32 0, i32 0
// CHECK-NEXT:    store i8 97, ptr [[I6]], align 8
// CHECK-NEXT:    call void @llvm.memcpy.p0.p0.i64(ptr align 8 [[TMP0]], ptr align 8 [[REF_TMP5]], i64 16, i1 false)
// CHECK-NEXT:    ret void
//
void foo1() {
  A a1(3.1, 2.0);
  B b1(A('c'));
  C c1(B(A(1, 1), 1), A('a'));
}


// CHECK-LABEL:   entry:
// CHECK-NEXT:    [[ARR1:%.*]] = alloca [4 x i32], align 16
// CHECK-NEXT:    [[ARR2:%.*]] = alloca [5 x i32], align 16
// CHECK-NEXT:    [[TMP0:%.*]] = getelementptr inbounds [4 x i32], ptr [[ARR1]], i64 0, i64 0
// CHECK-NEXT:    store i32 1, ptr [[TMP0]], align 16
// CHECK-NEXT:    [[TMP1:%.*]] = getelementptr inbounds [4 x i32], ptr [[ARR1]], i64 0, i64 1
// CHECK-NEXT:    store i32 2, ptr [[TMP1]], align 4
// CHECK-NEXT:    [[TMP2:%.*]] = getelementptr inbounds [4 x i32], ptr [[ARR1]], i64 0, i64 2
// CHECK-NEXT:    store i32 3, ptr [[TMP2]], align 8
// CHECK-NEXT:    [[TMP3:%.*]] = getelementptr inbounds [5 x i32], ptr [[ARR2]], i64 0, i64 0
// CHECK-NEXT:    store i32 0, ptr [[TMP3]], align 16
// CHECK-NEXT:    [[TMP4:%.*]] = getelementptr inbounds [5 x i32], ptr [[ARR2]], i64 0, i64 1
// CHECK-NEXT:    store i32 1, ptr [[TMP4]], align 4
// CHECK-NEXT:    [[TMP5:%.*]] = getelementptr inbounds [5 x i32], ptr [[ARR2]], i64 0, i64 2
// CHECK-NEXT:    store i32 2, ptr [[TMP5]], align 8
// CHECK-NEXT:    [[TMP6:%.*]] = getelementptr inbounds [5 x i32], ptr [[ARR2]], i64 0, i64 3
// CHECK-NEXT:    store i32 3, ptr [[TMP6]], align 4
// CHECK-NEXT:    [[TMP7:%.*]] = getelementptr inbounds [5 x i32], ptr [[ARR2]], i64 0, i64 4
// CHECK-NEXT:    store i32 4, ptr [[TMP7]], align 16
// CHECK-NEXT:    ret void
//
void foo2() {
  int arr1[4](1, 2, 3);
  int arr2[](0, 1, 2, 3, 4);
}
