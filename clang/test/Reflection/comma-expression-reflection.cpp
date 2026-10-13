// comma-expression-reflection.cpp — The comma operator reflects as a binary
// operator, reporting op_comma.
//
// RUN: %clang_cc1 -std=c++26 -freflection -fexpansion-statements %s -verify
// expected-no-diagnostics

#include <meta>

using namespace std::meta;

constexpr double sq(double x) { return x * x; }
constexpr double f(double x, double y) { return (sq(x), x * y); }

namespace test_comma {
  constexpr auto ret = return_value_of(statements_of(body_of(^^f))[0]);
  static_assert(is_expression(ret));
  static_assert(is_binary_operator(ret));
  static_assert(expression_operator_of(ret) == operators::op_comma);
  // Two operands: the discarded left (a call) and the result (a multiply).
  static_assert(operands_of(ret).size() == 2);
  static_assert(is_function_call(operands_of(ret)[0]));
  static_assert(is_binary_operator(operands_of(ret)[1]));
  static_assert(expression_operator_of(operands_of(ret)[1]) ==
                operators::op_star);
}

// A chain is left-associative: `(a, b, c)` is `((a, b), c)`.
constexpr double g(double x) { return ((void)x, (void)sq(x), x + 1.0); }

namespace test_chain {
  constexpr auto ret = return_value_of(statements_of(body_of(^^g))[0]);
  static_assert(is_binary_operator(ret));
  static_assert(expression_operator_of(ret) == operators::op_comma);
  static_assert(is_binary_operator(operands_of(ret)[0]));
  static_assert(expression_operator_of(operands_of(ret)[0]) ==
                operators::op_comma);
  static_assert(expression_operator_of(operands_of(ret)[1]) ==
                operators::op_plus);
}
