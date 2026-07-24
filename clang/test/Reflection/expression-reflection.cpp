// expression-reflection.cpp — Tests for ^^{ expr } expression reflection
//
// RUN: %clang_cc1 -std=c++26 -freflection %s -verify
// expected-no-diagnostics

#include <meta>

using namespace std::meta;

// Basic: ^^{ expr } produces a reflection
static_assert(is_expression(^^{ 1 + 2 }));
static_assert(!is_expression(^^int));

// Expression kind classification
static_assert(expression_kind_of(^^{ 42 }) == expr_kind::literal);
static_assert(expression_kind_of(^^{ 3.14 }) == expr_kind::literal);

namespace test_decl_ref {
  int x = 0;
  static_assert(expression_kind_of(^^{ x }) == expr_kind::decl_ref);
}

namespace test_binary_op {
  int a = 1, b = 2;
  static_assert(expression_kind_of(^^{ a + b }) == expr_kind::binary_op);
  static_assert(expression_kind_of(^^{ a * b }) == expr_kind::binary_op);
}

namespace test_unary_op {
  int x = 1;
  static_assert(expression_kind_of(^^{ -x }) == expr_kind::unary_op);
}

namespace test_call {
  int f(int);
  static_assert(expression_kind_of(^^{ f(1) }) == expr_kind::call);
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
//     the operand kind is `cast`. Strip the cast (first_operand_of) to reach
//     the underlying decl_ref, as real pattern-matching code must.
namespace test_operands {
  int a = 1, b = 2;
  static_assert(operands_of(^^{ a + b }).size() == 2);
  static_assert(expression_kind_of(operands_of(^^{ a + b })[0]) ==
                expr_kind::cast);
  static_assert(expression_kind_of(
                    first_operand_of(operands_of(^^{ a + b })[0])) ==
                expr_kind::decl_ref);
  static_assert(expression_kind_of(
                    first_operand_of(operands_of(^^{ a + b })[1])) ==
                expr_kind::decl_ref);
  static_assert(declaration_of(
                    first_operand_of(operands_of(^^{ a + b })[0])) == ^^a);
  static_assert(declaration_of(
                    first_operand_of(operands_of(^^{ a + b })[1])) == ^^b);
}

// Splice: [:^^{ expr }:] evaluates the expression
namespace test_splice {
  constexpr int val = [:^^{ 1 + 2 }:];
  static_assert(val == 3);

  constexpr int x = 10;
  constexpr int val2 = [:^^{ x * 2 }:];
  static_assert(val2 == 20);
}
