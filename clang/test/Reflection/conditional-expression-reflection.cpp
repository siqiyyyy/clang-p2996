// conditional-expression-reflection.cpp — Tests for reflection of the
// conditional (ternary) operator: is_conditional_operator, condition_of,
// true_expression_of, false_expression_of.
//
// RUN: %clang_cc1 -std=c++26 -freflection -fexpansion-statements %s -verify
// expected-no-diagnostics

#include <meta>

using namespace std::meta;

// Classification: a ternary is its own kind, not a binary operator.
namespace test_kind {
  int a = 1, b = 2;
  bool c = true;
  static_assert(is_expression(^^{ c ? a : b }));
  static_assert(is_conditional_operator(^^{ c ? a : b }));
  static_assert(!is_binary_operator(^^{ c ? a : b }));
  static_assert(!is_conditional_operator(^^{ a + b }));
  static_assert(!is_conditional_operator(^^int));
}

// Role accessors. Note that `c ? a : b` with two int lvalue operands is itself
// an lvalue, so the branches stay decl_refs; only the condition picks up the
// contextual conversion to bool (a cast).
namespace test_accessors {
  int a = 1, b = 2;
  bool c = true;
  static_assert(is_cast(condition_of(^^{ c ? a : b })));
  static_assert(declaration_of(operands_of(condition_of(^^{ c ? a : b }))[0]) ==
                ^^c);
  static_assert(is_variable_reference(true_expression_of(^^{ c ? a : b })));
  static_assert(declaration_of(true_expression_of(^^{ c ? a : b })) == ^^a);
  static_assert(declaration_of(false_expression_of(^^{ c ? a : b })) == ^^b);
}

// operands_of walks the same three subexpressions, in written order.
namespace test_operands {
  int a = 1, b = 2;
  bool c = true;
  static_assert(operands_of(^^{ c ? a : b }).size() == 3);
  static_assert(declaration_of(operands_of(^^{ c ? a : b })[1]) == ^^a);
  static_assert(declaration_of(operands_of(^^{ c ? a : b })[2]) == ^^b);
}

// Nesting: a branch may itself be a conditional.
namespace test_nested {
  int a = 1, b = 2;
  bool c = true;
  static_assert(
      is_conditional_operator(false_expression_of(^^{ c ? a : (c ? a : b) })));
  static_assert(declaration_of(false_expression_of(
                    false_expression_of(^^{ c ? a : (c ? a : b) }))) == ^^b);
}

// GNU `c ?: b`: the condition doubles as the true branch. Clang models the
// reuse with an OpaqueValueExpr; reflection reports the written operand.
namespace test_gnu_form {
  int a = 5, b = 2;
  static_assert(is_conditional_operator(^^{ a ?: b }));
  static_assert(declaration_of(condition_of(^^{ a ?: b })) == ^^a);
  static_assert(declaration_of(true_expression_of(^^{ a ?: b })) == ^^a);
  static_assert(declaration_of(false_expression_of(^^{ a ?: b })) == ^^b);
}

// Reached through a function body, as source-transformation code would.
namespace test_in_body {
  constexpr double f(double x, double y) { return x > y ? x - y : y - x; }
  static_assert(is_conditional_operator(
      return_value_of(statements_of(body_of(^^f))[0])));
  static_assert(expression_operator_of(condition_of(
      return_value_of(statements_of(body_of(^^f))[0]))) == operators::op_greater);
  static_assert(expression_operator_of(true_expression_of(
      return_value_of(statements_of(body_of(^^f))[0]))) == operators::op_minus);
  static_assert(expression_operator_of(false_expression_of(
      return_value_of(statements_of(body_of(^^f))[0]))) == operators::op_minus);
}

// Splice: a reflected ternary still evaluates.
namespace test_splice {
  constexpr int x = 3, y = 4;
  constexpr int v = [:^^{ x < y ? x : y }:];
  static_assert(v == 3);
}
