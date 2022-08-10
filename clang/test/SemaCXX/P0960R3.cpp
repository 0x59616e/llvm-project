// RUN: %clang_cc1 -verify -std=c++20 %s -fsyntax-only

struct A { // expected-note 4{{candidate constructor}}
  char i;
  double j;
};

struct B {
  A a;
  int b[20];
  int &&c; // expected-note {{reference member declared here}}
};

struct C { // expected-note 2{{candidate constructor}}
  A a;
  int b[20];
};

struct D : public C, public A {
  int a;
};

void foo() {
  A a(1954, 9, 21);
  // expected-error@-1 {{excess elements in struct initializer}}
  A b(2.1);
  // expected-warning@-1 {{implicit conversion from 'double' to 'char'}}
  A e(-1.2, 9.8);
  // expected-warning@-1 {{implicit conversion from 'double' to 'char'}}
  A s = static_cast<A>(1.1);
  // expected-warning@-1 {{implicit conversion from 'double' to 'char'}}
  A h = (A)3.1;
  // expected-warning@-1 {{implicit conversion from 'double' to 'char'}}
  A i = A(8.7);
  // expected-warning@-1 {{implicit conversion from 'double' to 'char'}}

  B n(2022, {7, 8});
  // expected-error@-1 {{no viable conversion from 'int' to 'A'}}
  B z(A(1), {}, 1);
  // expected-error@-1 {{reference member 'c' binds to a temporary object whose lifetime would be shorter than the lifetime of the constructed object}}

  C o(A(1), 1, 2, 3, 4);
  // expected-error@-1 {{excess elements in struct initializer}}
  D R(1);
  // expected-error@-1 {{no viable conversion from 'int' to 'C'}}
  D I(C(1));
  // expected-error@-1 {{functional-style cast from 'int' to 'C' is not allowed}}
  D P(C(A(1)), 1);
  // expected-error@-1 {{no viable conversion from 'int' to 'A'}}

  int arr1[](0, 1, 2, A(1));
  // expected-error@-1 {{no viable conversion from 'A' to 'int'}}
  int arr2[2](0, 1, 2);
  // expected-error@-1 {{excess elements in array initializer}}
}
