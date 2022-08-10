// RUN: %clang_cc1 -verify -std=c++20 %s -fsyntax-only

struct A { // expected-note 3{{candidate constructor}}
  char i;
  double j;
};

struct B { // expected-note 3{{candidate constructor}}
  A a;
  int b[20];
  int &&c; // expected-note {{reference member declared here}}
};

struct C { // expected-note 6{{candidate constructor}}
  A a;
  int b[20];
};

struct D : public C, public A { // expected-note 6{{candidate constructor}}
  int a;
};

void foo() {
  A a(1954, 9, 21);
  // expected-error@-1 {{no matching constructor for initialization of 'A'}}
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
  // expected-error@-1 {{no matching constructor for initialization}}
  B z(A(1), {}, 1);
  // expected-error@-1 {{reference member 'c' binds to a temporary object whose lifetime would be shorter than the lifetime of the constructed object}}

  C o(A(1), 1, 2, 3, 4);
  // expected-error@-1 {{no matching constructor for initialization of 'C'}}
  D R(1);
  // expected-error@-1 {{no matching constructor for initialization}}
  D I(C(1));
  // expected-error@-1 {{no matching conversion for functional-style cast from 'int' to 'C'}}
  D P(C(A(1)), 1);
  // expected-error@-1 {{no matching constructor for initialization}}

  int arr1[](0, 1, 2, A(1));
  // expected-error@-1 {{no viable conversion from 'A' to 'int'}}
  int arr2[2](0, 1, 2);
  // expected-error@-1 {{excess elements in array initializer}}
}
