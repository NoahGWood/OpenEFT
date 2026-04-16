// Copyright (c) 2026 Noah G. Wood
// SPDX-License-Identifier: LicenseRef-OpenEFT
//
// This file is part of OpenEFT.
// OpenEFT is source-available software. See LICENSE file for details.
#pragma once
#include <OpenEFT/rules/Rules.h>
namespace OpenEFT {

#define VALUE_CONSTRAINT_FIELDS(X) \
    X(numeric_value)               \
    X(enum_name)                   \
    X(enum_value)                  \
    X(pattern)

#define FIELD_REF_FIELDS(X) \
    X(record_type)          \
    X(field_id)

#define FIELD_CONDITION_FIELDS(X) \
    X(field)                      \
    X(value)

#define FIELD_CONDITION_GROUP_FIELDS(X) X(conditions)

#define FIELD_DEPENDENCY_RULE_FIELDS(X) \
    X(if_condition)                     \
    X(then_field)                       \
    X(then_value)

#define FIELD_GROUP_MEMBER_FIELDS(X) \
    X(record_type)                   \
    X(field_id)

#define FIELD_GROUP_RULE_FIELDS(X) X(members)

#define LINK_RULE_FIELDS(X) \
    X(source)               \
    X(target)               \
    X(must_exist)

#define CONDITIONAL_CONSTRAINT_FIELDS(X) \
    X(trigger)                           \
    X(target)                            \
    X(enforced)

#define ITEM_RULE_FIELDS(X) \
    X(item_name)            \
    X(item_short)           \
    X(min_length)           \
    X(max_length)           \
    X(constraints)

#define SUBFIELD_RULE_FIELDS(X) \
    X(subfield_name)            \
    X(subfield_short)           \
    X(subfield_id)              \
    X(min_occurs)               \
    X(max_occurs)               \
    X(items)

#define FIELD_RULE_FIELDS(X) \
    X(field_name)            \
    X(field_short)           \
    X(record_type)           \
    X(field_id)              \
    X(presence_conditions)   \
    X(min_length)            \
    X(max_length)            \
    X(min_occurs)            \
    X(max_occurs)            \
    X(subfields)             \
    X(constraints)

#define RECORD_RULE_FIELDS(X) \
    X(record_type)            \
    X(min_occurs)             \
    X(max_occurs)             \
    X(applies_when)           \
    X(fields)                 \
    X(dependencies)

#define RECORD_SET_RULE_FIELDS(X) \
    X(record_types)               \
    X(min_total)                  \
    X(max_total)

#define SEMANTIC_RULE_FIELDS(X) \
    X(target)                   \
    X(parameter)

#define TRANSACTION_RULE_FIELDS(X) \
    X(name)                        \
    X(transaction_type)            \
    X(domain_id)                   \
    X(records)                     \
    X(record_sets)                 \
    X(field_groups)                \
    X(link_rules)                  \
    X(conditional_constraints)     \
    X(semantic_rules)

}  // namespace OpenEFT
