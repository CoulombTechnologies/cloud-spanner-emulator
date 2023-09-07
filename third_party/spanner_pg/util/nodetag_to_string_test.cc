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

#include "third_party/spanner_pg/util/nodetag_to_string.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "zetasql/base/testing/status_matchers.h"
#include "third_party/spanner_pg/postgres_includes/all.h"

namespace postgres_translator {
namespace {

TEST(NodeTagToString, SingleValue) {
  EXPECT_EQ(NodeTagToString(T_Query), "T_Query");
}

TEST(NodeTagToString, InvalidValue) {
  EXPECT_EQ(NodeTagToString((NodeTag)16000), "<unknown:16000>");
}

TEST(NodeTagToString, AllValues) {
#define NODE(x) EXPECT_EQ(NodeTagToString(T_##x), "T_" #x);
#include "third_party/spanner_pg/postgres_includes/nodes.inc"
#undef NODE
}

TEST(NodeTagToNodeString, SingleValue) {
  EXPECT_EQ(NodeTagToNodeString(T_Query), "Query");
}

TEST(NodeTagToNodeString, InvalidValue) {
  EXPECT_EQ(NodeTagToNodeString((NodeTag)16000), "<unknown:16000>");
}

TEST(NodeTagToNodeString, AllValues) {
#define NODE(x) EXPECT_EQ(NodeTagToNodeString(T_##x), #x);
#include "third_party/spanner_pg/postgres_includes/nodes.inc"
#undef NODE
}

}  // namespace
}  // namespace postgres_translator
