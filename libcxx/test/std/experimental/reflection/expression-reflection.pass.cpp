//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++03 || c++11 || c++14 || c++17 || c++20
// ADDITIONAL_COMPILE_FLAGS: -freflection

// <experimental/reflection>
//
// [reflection]

// Tests for ^^{ expr } expression reflection

#include <meta>

using namespace std::meta;

// Basic: ^^{ expr } produces a reflection
static_assert(is_expression(^^{ 1 + 2 }));
static_assert(!is_expression(^^int));

// Expression classification predicates
static_assert(is_literal(^^{ 42 }));
static_assert(is_literal(^^{ 3.14 }));
static_assert(!is_binary_operator(^^{ 42 }));

namespace test_decl_ref {
  int x = 0;
  static_assert(is_variable_reference(^^{ x }));
  static_assert(!is_literal(^^{ x }));
}

namespace test_binary_op {
  int a = 1, b = 2;
  static_assert(is_binary_operator(^^{ a + b }));
  static_assert(is_binary_operator(^^{ a * b }));
  static_assert(!is_unary_operator(^^{ a + b }));
}

namespace test_unary_op {
  int x = 1;
  static_assert(is_unary_operator(^^{ -x }));
  static_assert(!is_binary_operator(^^{ -x }));
}

namespace test_call {
  int f(int);
  static_assert(is_function_call(^^{ f(1) }));
}

// declaration_of: extract the variable from a decl_ref
namespace test_declaration_of {
  int x = 0;
  int y = 0;
  static_assert(declaration_of(^^{ x }) == ^^x);
  static_assert(declaration_of(^^{ y }) == ^^y);
  static_assert(declaration_of(^^{ x }) != ^^y);
}

// operands_of: get children of binary_op.
//
// Two things to note:
//  1. operands_of returns a std::vector, which cannot be stored in a constexpr
//     variable (non-transient constexpr allocation is ill-formed). Call it
//     inline so the vector stays transient within each constant expression.
//  2. The captured AST includes implicit conversions: the operands of `a + b`
//     are lvalue-to-rvalue ImplicitCastExpr nodes wrapping the decl_refs, so
//     the operand is a cast. Strip the cast (its single operand) to reach
//     the underlying variable reference, as real pattern-matching code must.
namespace test_operands {
  int a = 1, b = 2;
  static_assert(operands_of(^^{ a + b }).size() == 2);
  static_assert(is_cast(operands_of(^^{ a + b })[0]));
  static_assert(
      is_variable_reference(operands_of(operands_of(^^{ a + b })[0])[0]));
  static_assert(
      is_variable_reference(operands_of(operands_of(^^{ a + b })[1])[0]));
  static_assert(declaration_of(operands_of(operands_of(^^{ a + b })[0])[0]) ==
                ^^a);
  static_assert(declaration_of(operands_of(operands_of(^^{ a + b })[1])[0]) ==
                ^^b);
}

// Splice: [:^^{ expr }:] evaluates the expression
namespace test_splice {
  constexpr int val = [:^^{ 1 + 2 }:];
  static_assert(val == 3);

  constexpr int x = 10;
  constexpr int val2 = [:^^{ x * 2 }:];
  static_assert(val2 == 20);
}

int main() {}
