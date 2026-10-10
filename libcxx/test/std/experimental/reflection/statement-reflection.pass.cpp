//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++03 || c++11 || c++14 || c++17 || c++20
// ADDITIONAL_COMPILE_FLAGS: -freflection -fexpansion-statements

// <experimental/reflection>
//
// [reflection]

// Tests for function/statement reflection:
// body_of, statements_of, the is_*_statement predicates, return_value_of,
// declared_variable_of, initializer_of.

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
static_assert(is_compound_statement(body_of(^^f)));

// statements_of walks the body's children.
static_assert(statements_of(body_of(^^f)).size() == 2);

// First child: the declaration `double a = x * y;`
static_assert(is_declaration_statement(statements_of(body_of(^^f))[0]));
static_assert(!is_return_statement(statements_of(body_of(^^f))[0]));
// Second child: the return statement.
static_assert(is_return_statement(statements_of(body_of(^^f))[1]));

// return_value_of gives an expression reflection (`a + x`, a binary_op).
namespace test_return {
  constexpr auto ret = return_value_of(statements_of(body_of(^^f))[1]);
  static_assert(is_expression(ret));
  static_assert(is_binary_operator(ret));
  static_assert(expression_operator_of(ret) == operators::op_plus);
}

// declared_variable_of -> the variable `a`; initializer_of(a) -> `x * y`.
namespace test_decl {
  constexpr auto s0 = statements_of(body_of(^^f))[0];
  constexpr auto va = declared_variable_of(s0);
  constexpr auto init = initializer_of(va);
  static_assert(is_expression(init));
  static_assert(is_binary_operator(init));
  static_assert(expression_operator_of(init) == operators::op_star);
}

// Expression-statement children come back as *statement* reflections (uniform
// statement model), classified by is_expression_statement. (<meta> has no
// public accessor from an expression statement to its expression.)
namespace test_expr_child {
  constexpr double g(double x) {
    x = x + 1.0;    // assignment expression statement
    return x * x;
  }
  // first child is the expression-statement `x = x + 1.0` (inlined because a
  // std::vector cannot be stored in a constexpr variable).
  static_assert(is_statement(statements_of(body_of(^^g))[0]));
  static_assert(!is_expression(statements_of(body_of(^^g))[0]));
  static_assert(is_expression_statement(statements_of(body_of(^^g))[0]));
  static_assert(!is_declaration_statement(statements_of(body_of(^^g))[0]));
}

int main() {}
