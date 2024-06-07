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

#ifndef INTERFACE_EMULATOR_BUILTIN_FUNCTION_CATALOG_
#define INTERFACE_EMULATOR_BUILTIN_FUNCTION_CATALOG_

#include <memory>
#include <string>

#include "zetasql/public/function.h"
#include "zetasql/public/types/type_factory.h"
#include "absl/status/statusor.h"
#include "backend/query/function_catalog.h"
#include "third_party/spanner_pg/interface/engine_builtin_function_catalog.h"
#include "zetasql/base/status_macros.h"

namespace postgres_translator {
namespace spangres {

// A wrapper around the Emulator catalog so that PG Spanner can access the
// functions that are built into Cloud Spanner.
class EmulatorBuiltinFunctionCatalog : public EngineBuiltinFunctionCatalog {
 public:
  explicit EmulatorBuiltinFunctionCatalog(
      std::unique_ptr<google::spanner::emulator::backend::FunctionCatalog>
          function_catalog)
      : EngineBuiltinFunctionCatalog(),
        function_catalog_(std::move(function_catalog)) {
  }

  ~EmulatorBuiltinFunctionCatalog() {
  }

  absl::StatusOr<const zetasql::Function*> GetFunction(
      const std::string& name) const override {
    const zetasql::Function* function;
    function_catalog_->GetFunction(name, &function);
    if (function == nullptr) {
      return absl::NotFoundError(absl::StrCat(name, " function not found"));
    }
    return function;
  }

  absl::StatusOr<const zetasql::Procedure*> GetProcedure(
      const std::string& name) const override {
    return absl::UnimplementedError("GetProcedure is not supported");
  }

  // TODO: b/313936285 - Add builtin TVF support to the Emulator.
  absl::StatusOr<const zetasql::TableValuedFunction*> GetTableValuedFunction(
      const std::string& name) const override {
    const zetasql::TableValuedFunction* tvf;
    function_catalog_->GetTableValuedFunction(name, &tvf);
    if (tvf == nullptr) {
      return absl::NotFoundError(
          absl::StrCat(name, " table valued function not found"));
    }
    return tvf;
  }

  absl::Status GetFunctions(
      absl::flat_hash_set<const zetasql::Function*>* output) const override {
    ZETASQL_RET_CHECK_NE(output, nullptr);
    ZETASQL_RET_CHECK(output->empty());
    function_catalog_->GetFunctions(output);
    return absl::OkStatus();
  }

  absl::Status GetProcedures(
      absl::flat_hash_set<const zetasql::Procedure*>* output) const override {
    return absl::OkStatus();
  }

  void SetLatestSchema(
      const google::spanner::emulator::backend::Schema* schema) {
    function_catalog_->SetLatestSchema(schema);
  }

  const google::spanner::emulator::backend::Schema* GetLatestSchema() {
    return function_catalog_->GetLatestSchema();
  }

 private:
  // The EmulatorBuiltinFunctionCatalog object must own this pointer; otherwise,
  // the returned function pointer from GetFunction() will become invalid and
  // point to an incorrect function.
  std::unique_ptr<google::spanner::emulator::backend::FunctionCatalog>
      function_catalog_;
};

}  // namespace spangres
}  // namespace postgres_translator

#endif  // INTERFACE_EMULATOR_BUILTIN_FUNCTION_CATALOG_
