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

#ifndef INTERFACE_EMULATOR_PARSER_H_
#define INTERFACE_EMULATOR_PARSER_H_

#include "zetasql/public/analyzer.h"
#include "zetasql/public/catalog.h"
#include "zetasql/public/types/type_factory.h"
#include "backend/query/function_catalog.h"
#include "third_party/spanner_pg/interface/spangres_translator_interface.h"

namespace postgres_translator {
namespace spangres {

absl::StatusOr<std::unique_ptr<const zetasql::AnalyzerOutput>>
ParseAndAnalyzePostgreSQL(
    const std::string& sql, zetasql::EnumerableCatalog* catalog,
    const zetasql::AnalyzerOptions& analyzer_options,
    zetasql::TypeFactory* type_factory,
    std::unique_ptr<google::spanner::emulator::backend::FunctionCatalog>
        emulator_function_catalog);

absl::StatusOr<interfaces::ExpressionTranslateResult>
TranslateTableLevelExpression(
    absl::string_view expression, absl::string_view table_name,
    zetasql::EnumerableCatalog& catalog,
    const zetasql::AnalyzerOptions& analyzer_options,
    zetasql::TypeFactory* type_factory,
    std::unique_ptr<google::spanner::emulator::backend::FunctionCatalog>
        emulator_function_catalog);

absl::StatusOr<interfaces::ExpressionTranslateResult> TranslateQueryInView(
    absl::string_view query, zetasql::EnumerableCatalog& catalog,
    const zetasql::AnalyzerOptions& analyzer_options,
    zetasql::TypeFactory* type_factory,
    std::unique_ptr<google::spanner::emulator::backend::FunctionCatalog>
        emulator_function_catalog);

}  // namespace spangres
}  // namespace postgres_translator

#endif  // INTERFACE_EMULATOR_PARSER_H_
