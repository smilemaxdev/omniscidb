/*
 * Copyright 2022 HEAVY.AI, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef QUERYENGINE_INPUTDESCRIPTORS_H
#define QUERYENGINE_INPUTDESCRIPTORS_H

#include "../Catalog/TableDescriptor.h"
#include "Logger/Logger.h"
#include "Shared/toString.h"

#include <memory>

enum class InputSourceType { TABLE, RESULT };

class InputDescriptor {
 public:
  InputDescriptor(const int table_id, const int nest_level)
      : table_id_(table_id), nest_level_(nest_level) {}

  bool operator==(const InputDescriptor& that) const {
    return table_id_ == that.table_id_ && nest_level_ == that.nest_level_;
  }

  int getTableId() const { return table_id_; }

  int getNestLevel() const { return nest_level_; }

  InputSourceType getSourceType() const {
    return table_id_ > 0 ? InputSourceType::TABLE : InputSourceType::RESULT;
  }

  size_t hash() const {
    static_assert(sizeof(table_id_) + sizeof(nest_level_) <= sizeof(size_t));
    return static_cast<size_t>(table_id_) << 8 * sizeof(nest_level_) |
           static_cast<size_t>(nest_level_);
  }

  std::string toString() const {
    return ::typeName(this) + "(table_id=" + std::to_string(table_id_) +
           ", nest_level=" + std::to_string(nest_level_) + ")";
  }

 private:
  int table_id_;
  int nest_level_;
};

inline std::ostream& operator<<(std::ostream& os, InputDescriptor const& id) {
  return os << "InputDescriptor(table_id(" << id.getTableId() << "),nest_level("
            << id.getNestLevel() << "))";
}

class InputColDescriptor final {
 public:
  InputColDescriptor(const int col_id, const int table_id, const int nest_level)
      : col_id_(col_id), input_desc_(table_id, nest_level) {}

  bool operator==(const InputColDescriptor& that) const {
    return col_id_ == that.col_id_ && input_desc_ == that.input_desc_;
  }

  int getColId() const { return col_id_; }

  const InputDescriptor& getScanDesc() const { return input_desc_; }

  size_t hash() const {
    return input_desc_.hash() ^ (static_cast<size_t>(col_id_) << 16);
  }

  std::string toString() const {
    return ::typeName(this) + "(col_id=" + std::to_string(col_id_) +
           ", input_desc=" + ::toString(input_desc_) + ")";
  }

 private:
  const int col_id_;
  const InputDescriptor input_desc_;
};

inline std::ostream& operator<<(std::ostream& os, InputColDescriptor const& icd) {
  return os << "InputColDescriptor(col_id(" << icd.getColId() << ")," << icd.getScanDesc()
            << ')';
}

// For printing RelAlgExecutionUnit::input_col_descs
inline std::ostream& operator<<(std::ostream& os,
                                std::shared_ptr<const InputColDescriptor> const& icd) {
  return os << *icd;
}

namespace std {
template <>
struct hash<InputColDescriptor> {
  size_t operator()(const InputColDescriptor& input_col_desc) const {
    return input_col_desc.hash();
  }
};

// Used by hash<std::shared_ptr<const InputColDescriptor>>.
template <>
struct hash<const InputColDescriptor*> {
  size_t operator()(const InputColDescriptor* input_col_desc) const {
    CHECK(input_col_desc);
    return input_col_desc->hash();
  }
};

template <>
struct equal_to<shared_ptr<const InputColDescriptor>> {
  bool operator()(shared_ptr<const InputColDescriptor> const& lhs,
                  shared_ptr<const InputColDescriptor> const& rhs) const {
    CHECK(lhs && rhs);
    return *lhs == *rhs;
  }
};
}  // namespace std

#endif  // QUERYENGINE_INPUTDESCRIPTORS_H
