// statement-reflection.cpp — Tests for function/statement reflection:
// body_of, statements_of, statement_kind_of, return_value_of,
// declared_variable_of, initializer_of.
//
// RUN: %clang_cc1 -std=c++26 -freflection -fexpansion-statements %s -verify
// expected-no-diagnostics

#include <meta>

using namespace std::meta;

// A straight-line function to reflect.
constexpr double f(double x, double y) {
  double a = x * y;
  return a + x;
}

// body_of yields a statement reflection (a compound statement).
static_assert(is_statement(body_of(^^f)));
static_assert(!is_expression(body_of(^^f)));
static_assert(statement_kind_of(body_of(^^f)) == stmt_kind::compound);

// statements_of walks the body's children.
static_assert(statements_of(body_of(^^f)).size() == 2);

// First child: the declaration `double a = x * y;`
static_assert(statement_kind_of(statements_of(body_of(^^f))[0]) ==
              stmt_kind::decl);
// Second child: the return statement.
static_assert(statement_kind_of(statements_of(body_of(^^f))[1]) ==
              stmt_kind::return_);

// return_value_of gives an expression reflection (`a + x`, a binary_op).
namespace test_return {
  constexpr auto ret = return_value_of(statements_of(body_of(^^f))[1]);
  static_assert(is_expression(ret));
  static_assert(expression_kind_of(ret) == expr_kind::binary_op);
  static_assert(expression_operator_of(ret) == operators::op_plus);
}

// declared_variable_of -> the variable `a`; initializer_of(a) -> `x * y`.
namespace test_decl {
  constexpr auto s0 = statements_of(body_of(^^f))[0];
  constexpr auto va = declared_variable_of(s0);
  constexpr auto init = initializer_of(va);
  static_assert(is_expression(init));
  static_assert(expression_kind_of(init) == expr_kind::binary_op);
  static_assert(expression_operator_of(init) == operators::op_star);
}

// Expression-statement children come back as *statement* reflections (uniform
// statement model), reporting stmt_kind::expression. expression_of() bridges to
// the underlying expression reflection, which then behaves as an expression.
namespace test_expr_child {
  constexpr double g(double x) {
    x = x + 1.0;    // assignment expression statement
    return x * x;
  }
  // first child is the expression-statement `x = x + 1.0` (inlined because a
  // std::vector cannot be stored in a constexpr variable).
  static_assert(is_statement(statements_of(body_of(^^g))[0]));
  static_assert(!is_expression(statements_of(body_of(^^g))[0]));
  static_assert(statement_kind_of(statements_of(body_of(^^g))[0]) ==
                stmt_kind::expression);
  // Cross into expression-context explicitly:
  static_assert(is_expression(expression_of(statements_of(body_of(^^g))[0])));
  static_assert(expression_kind_of(expression_of(statements_of(body_of(^^g))[0]))
                == expr_kind::binary_op);
}
