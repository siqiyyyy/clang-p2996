// for-statement-reflection.cpp — Tests for reflection of `for` statements:
// is_for_statement, init_statement_of, condition_of, increment_of,
// loop_body_of.
//
// RUN: %clang_cc1 -std=c++26 -freflection %s -verify
// expected-no-diagnostics

#include <meta>

using namespace std::meta;

// Classification: a `for` is its own statement kind.
namespace test_kind {
  constexpr double f() {
    double s = 0.0;
    for (int i = 0; i < 10; ++i)
      s = s + i;
    return s;
  }
  constexpr info fs = statements_of(body_of(^^f))[1];
  static_assert(is_statement(fs));
  static_assert(is_for_statement(fs));
  static_assert(!is_if_statement(fs));
  static_assert(!is_while_statement(fs));
  static_assert(!is_range_for_statement(fs));
}

// Full `for (init; cond; inc) body`, with an unbraced body.
namespace test_full {
  constexpr double f() {
    double s = 0.0;
    for (int i = 0; i < 10; ++i)
      s = s + i;
    return s;
  }
  constexpr info fs = statements_of(body_of(^^f))[1];
  constexpr info init = init_statement_of(fs);
  static_assert(init != info{});
  static_assert(is_declaration_statement(init));
  static_assert(identifier_of(declared_variable_of(init)) == "i");
  static_assert(is_expression(condition_of(fs)));
  static_assert(increment_of(fs) != info{});
  static_assert(is_statement(loop_body_of(fs)));
  static_assert(!is_compound_statement(loop_body_of(fs)));
  static_assert(is_expression_statement(loop_body_of(fs)));
}

// `for (;;)` -- every optional slot (init, condition, increment) absent.
namespace test_bare {
  constexpr int g() {
    for (;;) {
      return 1;
    }
  }
  constexpr info fs = statements_of(body_of(^^g))[0];
  static_assert(init_statement_of(fs) == info{});
  static_assert(condition_of(fs) == info{});
  static_assert(increment_of(fs) == info{});
  static_assert(is_compound_statement(loop_body_of(fs)));
}

// Nesting: a `for` inside a `for`.
namespace test_nested {
  constexpr double f() {
    double s = 0.0;
    for (int i = 0; i < 3; ++i) {
      for (int j = 0; j < 3; ++j) {
        s = s + 1.0;
      }
    }
    return s;
  }
  constexpr info outer = statements_of(body_of(^^f))[1];
  constexpr info inner = statements_of(loop_body_of(outer))[0];
  static_assert(is_for_statement(inner));
}
