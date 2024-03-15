//
// PostgreSQL is released under the PostgreSQL License, a liberal Open Source
// license, similar to the BSD or MIT licenses.
//
// PostgreSQL Database Management System
// (formerly known as Postgres, then as Postgres95)
//
// Portions Copyright © 1996-2020, The PostgreSQL Global Development Group
//
// Portions Copyright © 1994, The Regents of the University of California
//
// Portions Copyright 2023 Google LLC
//
// Permission to use, copy, modify, and distribute this software and its
// documentation for any purpose, without fee, and without a written agreement
// is hereby granted, provided that the above copyright notice and this
// paragraph and the following two paragraphs appear in all copies.
//
// IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY FOR
// DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
// LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
// EVEN IF THE UNIVERSITY OF CALIFORNIA HAS BEEN ADVISED OF THE POSSIBILITY OF
// SUCH DAMAGE.
//
// THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
// INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
// FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON AN
// "AS IS" BASIS, AND THE UNIVERSITY OF CALIFORNIA HAS NO OBLIGATIONS TO PROVIDE
// MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
//------------------------------------------------------------------------------

#include "third_party/spanner_pg/interface/stub_builtin_function_catalog.h"

#include "zetasql/public/analyzer.h"
#include "zetasql/public/function.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "zetasql/base/testing/status_matchers.h"
#include "absl/status/statusor.h"
#include "zetasql/base/status_macros.h"

namespace postgres_translator {

namespace {

TEST(StubBuiltinFunctionCatalogTest, GetFunction) {
  StubBuiltinFunctionCatalog catalog =
      StubBuiltinFunctionCatalog(zetasql::LanguageOptions());
  ZETASQL_ASSERT_OK_AND_ASSIGN(const zetasql::Function* function,
                       catalog.GetFunction("$add"));
  EXPECT_NE(function, nullptr);
  ZETASQL_ASSERT_OK_AND_ASSIGN(function,
                       catalog.GetFunction("unknown_function"));
  EXPECT_EQ(function, nullptr);
}

TEST(StubBuiltinFunctionCatalogTest, GetFunctions) {
  StubBuiltinFunctionCatalog catalog =
      StubBuiltinFunctionCatalog(zetasql::LanguageOptions());

  absl::flat_hash_set<const zetasql::Function*> all_functions;
  ZETASQL_ASSERT_OK(catalog.GetFunctions(&all_functions));
  EXPECT_GT(all_functions.size(), 1);
}

}  // namespace

}  // namespace postgres_translator
