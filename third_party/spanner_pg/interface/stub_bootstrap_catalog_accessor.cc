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

#include "third_party/spanner_pg/interface/bootstrap_catalog_accessor.h"

#include "absl/status/status.h"
#include "absl/status/statusor.h"

namespace postgres_translator {

const PgBootstrapCatalog* GetPgBootstrapCatalog() {
  return nullptr;
}

absl::StatusOr<PgCollationData> GetPgCollationDataFromBootstrap(
    const PgBootstrapCatalog* catalog, absl::string_view collation_name) {
  return absl::UnimplementedError(
      "invoked stub GetPgCollationDataFromBootstrap");
}

absl::StatusOr<PgNamespaceData> GetPgNamespaceDataFromBootstrap(
    const PgBootstrapCatalog* catalog, absl::string_view namespace_name) {
  return absl::UnimplementedError(
      "invoked stub GetPgNamespaceDataFromBootstrap");
}

absl::StatusOr<PgProcData> GetPgProcDataFromBootstrap(
    const PgBootstrapCatalog* catalog, int64_t proc_oid) {
  return absl::UnimplementedError("invoked stub GetPgProcDataFromBootstrap");
}

absl::StatusOr<PgTypeData> GetPgTypeDataFromBootstrap(
    const PgBootstrapCatalog* catalog, absl::string_view type_name) {
  return absl::UnimplementedError("invoked stub GetPgTypeDataFromBootstrap");
}

absl::StatusOr<PgTypeData> GetPgTypeDataFromBootstrap(
    const PgBootstrapCatalog* catalog, int64_t type_oid) {
  return absl::UnimplementedError("invoked stub GetPgTypeDataFromBootstrap");
}


}  // namespace postgres_translator
