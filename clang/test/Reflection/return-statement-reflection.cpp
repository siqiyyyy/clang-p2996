// return-statement-reflection.cpp — end-to-end test for ^^{ return expr; }
//
// Builds an executable and runs it. The reflection queries are consteval, so
// they are evaluated at compile time inside a constexpr context (std::meta::info
// is a consteval-only type and cannot appear in a run-time expression). Their
// boolean results are then checked at run time via assert(), so a zero exit
// status means every query produced the expected answer.
//
// RUN: %clang -std=c++26 -freflection %s -o %t
// RUN: %t

#include <meta>
#include <cassert>

using namespace std::meta;

int a = 1, b = 2;
int x = 0;

int main() {
  // A compound returned expression exposes its operands. Each operand of
  // 'a + b' is an lvalue-to-rvalue cast wrapping a decl_ref.
  constexpr auto add = return_value_of(^^{ return a + b; });
  constexpr auto op0 = first_operand_of(add);
  constexpr auto op1 = next_operand_of(add, op0);

  // Every reflection fact is computed at compile time here...
  constexpr bool checks[] = {
    // Classification: ^^{ return expr; } is a return statement; other kinds
    // of reflection are not.
    is_return_statement(^^{ return x; }),
    !is_return_statement(^^{ ++x }),  // an expression
    !is_return_statement(^^int),      // a type
    !is_return_statement(^^x),        // a declaration

    // return_value_of yields the returned expression as an expression
    // reflection that can be introspected with the expression API.
    is_expression(return_value_of(^^{ return x; })),
    expression_kind_of(return_value_of(^^{ return x; })) == expr_kind::decl_ref,
    declaration_of(return_value_of(^^{ return x; })) == ^^x,

    // A literal return value classifies as a literal expression.
    expression_kind_of(return_value_of(^^{ return 42; })) == expr_kind::literal,

    // Operands of the returned 'a + b' expression.
    expression_kind_of(add) == expr_kind::binary_op,
    expression_kind_of(op0) == expr_kind::cast,
    expression_kind_of(op1) == expr_kind::cast,
    declaration_of(first_operand_of(op0)) == ^^a,
    declaration_of(first_operand_of(op1)) == ^^b,

    // A bare 'return;' is still a return statement, but has no operand:
    // return_value_of yields a null (non-expression) reflection.
    is_return_statement(^^{ return; }),
    !is_expression(return_value_of(^^{ return; })),
  };

  // ...and verified at run time.
  for (bool ok : checks)
    assert(ok);

  return 0;
}
