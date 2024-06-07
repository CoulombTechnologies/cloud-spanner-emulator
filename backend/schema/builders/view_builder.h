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

#ifndef THIRD_PARTY_CLOUD_SPANNER_EMULATOR_BACKEND_SCHEMA_BUILDERS_VIEW_BUILDER_H_
#define THIRD_PARTY_CLOUD_SPANNER_EMULATOR_BACKEND_SCHEMA_BUILDERS_VIEW_BUILDER_H_

#include <algorithm>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "absl/memory/memory.h"
#include "backend/common/ids.h"
#include "backend/schema/catalog/view.h"
#include "backend/schema/validators/view_validator.h"

namespace google {
namespace spanner {
namespace emulator {
namespace backend {

class View::Builder {
 public:
  Builder()
      : instance_(absl::WrapUnique(new View(ViewValidator::Validate,
                                            ViewValidator::ValidateUpdate))) {}

  std::unique_ptr<const View> build() { return std::move(instance_); }

  const View* get() const { return instance_.get(); }

  Builder& set_name(absl::string_view name) {
    instance_->name_ = name;
    return *this;
  }

  Builder& set_sql_security(const View::SqlSecurity& security) {
    instance_->security_ = security;
    return *this;
  }

  Builder& set_sql_body(absl::string_view body) {
    instance_->body_ = body;
    return *this;
  }

  Builder& set_sql_body_origin(absl::string_view body_origin) {
    instance_->body_origin_ = body_origin;
    return *this;
  }

  Builder& add_column(View::Column column) {
    instance_->columns_.push_back(column);
    instance_->columns_map_.emplace(column.name, column);
    return *this;
  }

  Builder& add_dependency(const SchemaNode* dependency) {
    instance_->dependencies_.push_back(dependency);
    return *this;
  }

  Builder& set_postgresql_oid(std::optional<uint32_t> postgresql_oid) {
    if (postgresql_oid.has_value()) {
      instance_->set_postgresql_oid(postgresql_oid.value());
    }
    return *this;
  }

 private:
  std::unique_ptr<View> instance_;
};

class View::Editor {
 public:
  explicit Editor(View* instance) : instance_(instance) {}

  const View* get() const { return instance_; }

  // The only kind of 'edit' possible on a view is a complete
  // replacement of its definition.
  Editor& copy_from(const View* view) {
    instance_->name_ = view->name_;
    instance_->columns_.clear();
    for (const auto& it : view->columns_) {
      instance_->columns_.push_back(it);
    }
    instance_->columns_map_.clear();
    for (const auto& it : view->columns_map_) {
      instance_->columns_map_.emplace(it.first, it.second);
    }

    instance_->security_ = view->security_;
    instance_->body_ = view->body_;
    instance_->body_origin_ = view->body_origin_;
    instance_->dependencies_ = view->dependencies_;
    return *this;
  }

 private:
  // Not owned.
  View* instance_;
};

}  // namespace backend
}  // namespace emulator
}  // namespace spanner
}  // namespace google
#endif  // THIRD_PARTY_CLOUD_SPANNER_EMULATOR_BACKEND_SCHEMA_BUILDERS_VIEW_BUILDER_H_
