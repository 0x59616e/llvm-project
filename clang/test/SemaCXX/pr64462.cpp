// RUN: %clang_cc1 -std=c++20 -fsyntax-only -verify %s

auto c1(auto f, auto ...fs) {
    // expected-note@+4{{candidate template ignored: failed template argument deduction}}
    // expected-error@+3{{atomic constraint must be of type 'bool'}}
    // expected-note@+2{{because substituted constraint expression is ill-formed: no matching function for call to 'c1'}}
    // expected-note@+1{{candidate template ignored: constraints not satisfied}}
    return [](auto) requires (c1(fs...)) {};
}

auto c2(auto f, auto ...fs) {
    // expected-note@-1 2{{declared here}}
    // expected-note@+4 2{{function parameter 'f' with unknown value cannot be used in a constant expression}}
    // expected-note@+3 2{{while checking constraint satisfaction for function 'operator()' required here}}
    // expected-error@+2 2{{substitution into constraint expression resulted in a non-constant expression}}
    // expected-note@+1{{candidate function not viable: constraints not satisfied}}
    return []() requires f {};
}

auto c3(auto) {
    constexpr bool b = false;
    // expected-note@+2{{because 'b' evaluated to false}}
    // expected-note@+1{{candidate function not viable: constraints not satisfied}}
    return []() requires b {};
}

template<typename>
constexpr bool alwaysFalse() { return false; }

auto c4(auto t) {
    // expected-note@+2{{because 'alwaysFalse<decltype(t)>()' evaluated to false}}
    // expected-note@+1{{candidate function not viable: constraints not satisfied}}
    return []() requires (alwaysFalse<decltype(t)>()) {};
}

auto c5(auto t) {
    constexpr bool a = true;
    return [&]() {
        constexpr bool b = true;
        return [&]() {
            constexpr bool c = true;
            // expected-note@+3 {{candidate function not viable: constraints not satisfied}}
            // expected-note@+2 2{{while checking constraint satisfaction for function 'operator()' required here}}
            // expected-error@+1 2{{atomic constraint must be of type 'bool' (found '<overloaded function type>')}}
            return []() requires a && b && c && (alwaysFalse<decltype(t)>) {};
        }();
    }();
}

auto c6(auto t) { // expected-note {{declared here}}
    constexpr bool a = true;
    return [](auto) requires a {
        constexpr bool b = true;
        return []() requires a && b {
            // expected-note@+3{{candidate template ignored: failed template argument deduction}}
            // expected-note@+2{{function parameter 't' with unknown value cannot be used in a constant expression}}
            // expected-error@+1{{substitution into constraint expression resulted in a non-constant expression}}
            return [](auto) requires a && b && t {};
        };
    };
}


void bar() {
    c1(true)(true); // expected-error{{no matching function for call to object of type '(lambda at}}
    // expected-error@+3{{no matching function for call to object of type}}
    // expected-note@+2{{in instantiation of function template specialization 'c1(bool, bool)::(anonymous class)::operator()<bool>' requested here}}
    // expected-note@+1{{while checking constraint satisfaction for template 'operator()<bool, bool>' required here}}
    c1(true, true)(true);
    c2(true)(); // expected-error{{no matching function for call to object of type '(lambda at}}
    c3(true)(); // expected-error{{no matching function for call to object of type '(lambda at}}
    c4(true)(); // expected-error{{no matching function for call to object of type '(lambda}}
    c5(true)(); // expected-error{{no matching function for call to object of type '(lambda}}
    // expected-error@+3{{no matching function for call to object of type '(lambda at}}
    // expected-note@+2{{in instantiation of function template specialization 'c6(bool)::(anonymous class)::operator()(bool)::(anonymous class)::operator()()::(anonymous class)::operator()<bool>' requested here}}
    // expected-note@+1{{while checking constraint satisfaction for template 'operator()<bool>' required here}}
    c6(true)(true)()(true);
}
