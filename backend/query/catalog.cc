//
// Copyright 2020 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include "backend/query/catalog.h"

#include <memory>
#include <optional>
#include <string>
#include <utility>

#include "zetasql/public/catalog.h"
#include "zetasql/public/function.h"
#include "zetasql/public/types/type.h"
#include "zetasql/public/types/type_factory.h"
#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/memory/memory.h"
#include "absl/status/status.h"
#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "absl/synchronization/mutex.h"
#include "backend/access/read.h"
#include "backend/query/analyzer_options.h"
#include "backend/query/change_stream/queryable_change_stream_tvf.h"
#include "backend/query/function_catalog.h"
#include "backend/query/information_schema_catalog.h"
#include "backend/query/queryable_table.h"
#include "backend/query/queryable_view.h"
#include "backend/query/spanner_sys_catalog.h"
#include "backend/schema/catalog/schema.h"
#include "common/errors.h"
#include "third_party/spanner_pg/catalog/pg_catalog.h"
#include "absl/status/status.h"

namespace google {
namespace spanner {
namespace emulator {
namespace backend {

// A sub-catalog used for resolving NET function lookups.
class NetCatalog : public zetasql::Catalog {
 public:
  explicit NetCatalog(backend::Catalog* root_catalog)
      : root_catalog_(root_catalog) {}

  static constexpr char kName[] = "NET";

  std::string FullName() const final {
    std::string name = root_catalog_->FullName();
    if (name.empty()) {
      return kName;
    }
    absl::StrAppend(&name, kName);
    return name;
  }

  // Implementation of the zetasql::Catalog interface.
  absl::Status GetFunction(const std::string& name,
                           const zetasql::Function** function,
                           const FindOptions& options) final {
    // The list of all functions is maintained in the root catalog in the
    // form of their fully-qualified names. Just prefix the function name
    // with the name of 'this' catalog and delegate the request to parent.
    return root_catalog_->GetFunction(absl::StrJoin({FullName(), name}, "."),
                                      function, options);
  }

 private:
  backend::Catalog* root_catalog_;
};

// A sub-catalog used for resolving PG function lookups from GSQL queries.
// Required for supporting check constraints as PG queries are translated to
// GSQL queries before storing in the DDL statement.
class PGFunctionCatalog : public zetasql::Catalog {
 public:
  explicit PGFunctionCatalog(backend::Catalog* root_catalog)
      : root_catalog_(root_catalog) {}

  static constexpr char kName[] = "PG";

  std::string FullName() const final {
    std::string name = root_catalog_->FullName();
    if (name.empty()) {
      return kName;
    }
    absl::StrAppend(&name, kName);
    return name;
  }

  // Implementation of the zetasql::Catalog interface.
  absl::Status GetFunction(const std::string& name,
                           const zetasql::Function** function,
                           const FindOptions& options) final {
    // The list of all functions is maintained in the root catalog in the
    // form of their fully-qualified names. Just prefix the function name
    // with the name of 'this' catalog and delegate the request to parent.
    return root_catalog_->GetFunction(absl::StrJoin({FullName(), name}, "."),
                                      function, options);
  }

 private:
  backend::Catalog* root_catalog_;
};

Catalog::Catalog(const Schema* schema, const FunctionCatalog* function_catalog,
                 zetasql::TypeFactory* type_factory,
                 const zetasql::AnalyzerOptions& options, RowReader* reader,
                 QueryEvaluator* query_evaluator,
                 std::optional<std::string> change_stream_internal_lookup)
    : schema_(schema),
      function_catalog_(function_catalog),
      type_factory_(type_factory) {
  // Pass the reader to tables.
  for (const auto* table : schema->tables()) {
    tables_[table->Name()] = std::make_unique<QueryableTable>(
        table, reader, options, this, type_factory);
  }

  // Pass the query_evaluator to views.
  for (const auto* view : schema->views()) {
    views_[view->Name()] =
        std::make_unique<QueryableView>(view, query_evaluator);
  }

  if (change_stream_internal_lookup.has_value()) {
    auto change_stream =
        schema->FindChangeStream(change_stream_internal_lookup.value());
    auto partition_table = change_stream->change_stream_partition_table();
    auto data_table = change_stream->change_stream_data_table();
    tables_[partition_table->Name()] = std::make_unique<QueryableTable>(
        partition_table, reader, options, this, type_factory);
    tables_[data_table->Name()] = std::make_unique<QueryableTable>(
        data_table, reader, options, this, type_factory);
  }
  // Register a table valued function for each active change stream
  for (const auto* change_stream : schema->change_streams()) {
    tvfs_[change_stream->tvf_name()] =
        std::move(*QueryableChangeStreamTvf::Create(
            change_stream->tvf_name(), options, this, type_factory,
            schema->dialect() == database_api::DatabaseDialect::POSTGRESQL));
  }
}

absl::Status Catalog::GetCatalog(const std::string& name,
                                 zetasql::Catalog** catalog,
                                 const FindOptions& options) {
  if (absl::EqualsIgnoreCase(name, InformationSchemaCatalog::kName)) {
    *catalog = GetInformationSchemaCatalog();
  } else if (absl::EqualsIgnoreCase(name, InformationSchemaCatalog::kPGName)) {
    *catalog = GetPGInformationSchemaCatalog();
  } else if (absl::EqualsIgnoreCase(name, SpannerSysCatalog::kName)) {
    *catalog = GetSpannerSysCatalog();
  } else if (absl::EqualsIgnoreCase(name, NetCatalog::kName)) {
    *catalog = GetNetFunctionsCatalog();
  } else if (absl::EqualsIgnoreCase(name, PGFunctionCatalog::kName)) {
    *catalog = GetPGFunctionsCatalog();
  } else if (absl::EqualsIgnoreCase(name,
                                    postgres_translator::PGCatalog::kName)) {
    *catalog = GetPGCatalog();
  }
  return absl::OkStatus();
}

absl::Status Catalog::GetTable(const std::string& name,
                               const zetasql::Table** table,
                               const FindOptions& options) {
  *table = nullptr;
  if (auto it = views_.find(name); it != views_.end()) {
    *table = it->second.get();
    return absl::OkStatus();
  }

  if (auto it = tables_.find(name); it != tables_.end()) {
    *table = it->second.get();
    return absl::OkStatus();
  }

  return error::TableNotFound(name);
}

absl::Status Catalog::GetTableValuedFunction(
    const std::string& name, const zetasql::TableValuedFunction** tvf,
    const FindOptions& options) {
  *tvf = nullptr;
  if (auto it = tvfs_.find(name); it != tvfs_.end()) {
    *tvf = it->second.get();
    return absl::OkStatus();
  }
  return error::TableValuedFunctionNotFound(name);
}

absl::Status Catalog::GetFunction(const std::string& name,
                                  const zetasql::Function** function,
                                  const FindOptions& options) {
  function_catalog_->GetFunction(name, function);
  return absl::OkStatus();
}

absl::Status Catalog::GetCatalogs(
    absl::flat_hash_set<const zetasql::Catalog*>* output) const {
  output->insert(GetInformationSchemaCatalog());
  output->insert(GetSpannerSysCatalog());
  output->insert(GetNetFunctionsCatalog());
  return absl::OkStatus();
}

absl::Status Catalog::GetTables(
    absl::flat_hash_set<const zetasql::Table*>* output) const {
  for (auto iter = tables_.begin(); iter != tables_.end(); ++iter) {
    output->insert(iter->second.get());
  }
  for (auto iter = views_.begin(); iter != views_.end(); ++iter) {
    output->insert(iter->second.get());
  }
  return absl::OkStatus();
}

absl::Status Catalog::GetTypes(
    absl::flat_hash_set<const zetasql::Type*>* output) const {
  // Currently, Cloud Spanner doesn't support proto or enum types.
  return absl::OkStatus();
}

absl::Status Catalog::GetFunctions(
    absl::flat_hash_set<const zetasql::Function*>* output) const {
  function_catalog_->GetFunctions(output);
  return absl::OkStatus();
}

zetasql::Catalog* Catalog::GetInformationSchemaCatalog() const {
  absl::MutexLock lock(&mu_);
  auto spanner_sys_catalog = GetSpannerSysCatalogWithoutLocks();
  if (!information_schema_catalog_) {
    information_schema_catalog_ = std::make_unique<InformationSchemaCatalog>(
        InformationSchemaCatalog::kName, schema_, spanner_sys_catalog);
  }
  return information_schema_catalog_.get();
}

SpannerSysCatalog* Catalog::GetSpannerSysCatalog() const {
  absl::MutexLock lock(&mu_);
  return GetSpannerSysCatalogWithoutLocks();
}

SpannerSysCatalog* Catalog::GetSpannerSysCatalogWithoutLocks() const {
  if (!spanner_sys_catalog_) {
    spanner_sys_catalog_ = std::make_unique<SpannerSysCatalog>();
  }
  return spanner_sys_catalog_.get();
}

zetasql::Catalog* Catalog::GetPGInformationSchemaCatalog() const {
  absl::MutexLock lock(&mu_);
  auto spanner_sys_catalog = GetSpannerSysCatalogWithoutLocks();
  if (!pg_information_schema_catalog_) {
    pg_information_schema_catalog_ = std::make_unique<InformationSchemaCatalog>(
        InformationSchemaCatalog::kPGName, schema_, spanner_sys_catalog);
  }
  return pg_information_schema_catalog_.get();
}

zetasql::Catalog* Catalog::GetNetFunctionsCatalog() const {
  absl::MutexLock lock(&mu_);
  if (!net_catalog_) {
    net_catalog_ = std::make_unique<NetCatalog>(const_cast<Catalog*>(this));
  }
  return net_catalog_.get();
}

zetasql::Catalog* Catalog::GetPGFunctionsCatalog() const {
  absl::MutexLock lock(&mu_);
  if (!pg_function_catalog_) {
    pg_function_catalog_ =
        std::make_unique<PGFunctionCatalog>(const_cast<Catalog*>(this));
  }
  return pg_function_catalog_.get();
}

zetasql::Catalog* Catalog::GetPGCatalog() const {
  absl::MutexLock lock(&mu_);
  if (!pg_catalog_) {
    pg_catalog_ = std::make_unique<postgres_translator::PGCatalog>(schema_);
  }
  return pg_catalog_.get();
}

}  // namespace backend
}  // namespace emulator
}  // namespace spanner
}  // namespace google
