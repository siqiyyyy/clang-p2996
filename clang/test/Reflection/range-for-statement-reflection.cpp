// range-for-statement-reflection.cpp — Tests for reflection of range-based
// `for` statements: is_range_for_statement, range_init_of, loop_variable_of,
// loop_body_of.
//
// RUN: %clang_cc1 -std=c++26 -freflection %s -verify
// expected-no-diagnostics

#include <meta>

using namespace std::meta;

// Classification: a range-for is its own statement kind, distinct from a
// classic `for`.
namespace test_kind {
  constexpr double f() {
    double arr[3] = {1.0, 2.0, 3.0};
    double s = 0.0;
    for (const auto& x : arr) {
      s = s + x;
    }
    return s;
  }
  constexpr info rfs = statements_of(body_of(^^f))[2];
  static_assert(is_statement(rfs));
  static_assert(is_range_for_statement(rfs));
  static_assert(!is_for_statement(rfs));
}

// Role accessors: the range, the loop variable, and the (braced) body.
namespace test_accessors {
  constexpr double f() {
    double arr[3] = {1.0, 2.0, 3.0};
    double s = 0.0;
    for (const auto& x : arr) {
      s = s + x;
    }
    return s;
  }
  constexpr info rfs = statements_of(body_of(^^f))[2];
  static_assert(is_expression(range_init_of(rfs)));
  static_assert(identifier_of(loop_variable_of(rfs)) == "x");
  static_assert(is_compound_statement(loop_body_of(rfs)));
}

// Unbraced body is a single statement, not a compound one.
namespace test_unbraced {
  constexpr double f() {
    double arr[3] = {1.0, 2.0, 3.0};
    double s = 0.0;
    for (const auto& x : arr)
      s = s + x;
    return s;
  }
  constexpr info rfs = statements_of(body_of(^^f))[2];
  static_assert(!is_compound_statement(loop_body_of(rfs)));
  static_assert(is_expression_statement(loop_body_of(rfs)));
}
