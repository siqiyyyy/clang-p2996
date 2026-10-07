// if-statement-reflection.cpp — Tests for reflection of `if` statements:
// is_if_statement, init_statement_of, condition_of, then_statement_of,
// else_statement_of.
//
// RUN: %clang_cc1 -std=c++26 -freflection %s -verify
// expected-no-diagnostics

#include <meta>

using namespace std::meta;

// Classification: an `if` is its own statement kind.
namespace test_kind {
  constexpr double f(double x) {
    if (x > 0.0)
      return x;
    return -x;
  }
  constexpr info ifs = statements_of(body_of(^^f))[0];
  static_assert(is_statement(ifs));
  static_assert(is_if_statement(ifs));
  static_assert(!is_for_statement(ifs));
  static_assert(!is_while_statement(ifs));
  static_assert(!is_range_for_statement(ifs));
}

// Plain `if (cond) then` with no else and no init-statement: both
// init_statement_of and else_statement_of are null. The then-branch is a
// single, unbraced return statement -- not a compound statement.
namespace test_plain {
  constexpr double f(double x) {
    if (x > 0.0)
      return x;
    return -x;
  }
  constexpr info ifs = statements_of(body_of(^^f))[0];
  static_assert(init_statement_of(ifs) == info{});
  static_assert(else_statement_of(ifs) == info{});
  static_assert(is_expression(condition_of(ifs)));
  static_assert(is_statement(then_statement_of(ifs)));
  static_assert(!is_compound_statement(then_statement_of(ifs)));
  static_assert(is_return_statement(then_statement_of(ifs)));
}

// `if (cond) {...} else {...}`: else_statement_of is present and braced.
namespace test_else {
  constexpr double f(double x) {
    if (x > 0.0) {
      return x;
    } else {
      return -x;
    }
  }
  constexpr info ifs = statements_of(body_of(^^f))[0];
  static_assert(else_statement_of(ifs) != info{});
  static_assert(is_compound_statement(then_statement_of(ifs)));
  static_assert(is_compound_statement(else_statement_of(ifs)));
}

// `if (init; cond)`: init_statement_of gives the declaration.
namespace test_init {
  constexpr double f(double x) {
    if (double y = x * x; y > 1.0)
      return y;
    return 0.0;
  }
  constexpr info ifs = statements_of(body_of(^^f))[0];
  constexpr info init = init_statement_of(ifs);
  static_assert(init != info{});
  static_assert(is_declaration_statement(init));
  static_assert(identifier_of(declared_variable_of(init)) == "y");
}

// Nesting: an `if` inside the then-branch of another `if`.
namespace test_nested {
  constexpr double f(double x, double y) {
    if (x > 0.0) {
      if (y > 0.0)
        return x + y;
      return x - y;
    }
    return -x;
  }
  constexpr info outer = statements_of(body_of(^^f))[0];
  constexpr info inner = statements_of(then_statement_of(outer))[0];
  static_assert(is_if_statement(inner));
  static_assert(else_statement_of(inner) == info{});
}
