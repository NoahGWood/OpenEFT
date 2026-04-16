// Copyright (c) 2026 Noah G. Wood
// SPDX-License-Identifier: LicenseRef-OpenEFT
//
// This file is part of OpenEFT.
// OpenEFT is source-available software. See LICENSE file for details.
#pragma once
#include <NMTK/NMTK.h>
#include <OpenEFT/rules/EnumHelpers.h>
#include <OpenEFT/rules/EnumRegistry.h>
#include <OpenEFT/rules/RuleMacros.h>
#include <OpenEFT/rules/Rules.h>
#include <OpenEFT/rules/RulesUIEnums.h>

#include <string>
#include <type_traits>

namespace OpenEFT {
    template <typename Enum>
    inline std::enable_if_t<std::is_enum_v<Enum>> DrawFieldUI(const char* label,
                                                              Enum& value) {
        DrawEnumUI(label, value);
    }

    inline void DrawFieldUI(const char* label, int& v) {
        ImGui::InputInt(label, &v);
    }

    inline void DrawFieldUI(const char* label, uint32_t& v) {
        int tmp = static_cast<int>(v);
        if (ImGui::InputInt(label, &tmp)) {
            v = static_cast<uint32_t>(tmp);
        }
    }

    inline void DrawFieldUI(const char* label, uint16_t& v) {
        int tmp = static_cast<int>(v);
        if (ImGui::InputInt(label, &tmp)) {
            v = static_cast<uint16_t>(tmp);
        }
    }
    inline void DrawFieldUI(const char* label, int64_t& v) {
        ImGui::InputScalar(label, ImGuiDataType_S64, &v);
    }
    inline void DrawFieldUI(const char* label, bool& v) {
        ImGui::Checkbox(label, &v);
    }
    inline void DrawFieldUI(const char* label, std::string& v) {
        char buffer[512];
        strncpy(buffer, v.c_str(), sizeof(buffer));

        if (ImGui::InputText(label, buffer, sizeof(buffer))) {
            v = buffer;
        }
    }

    template <typename Enum>
    inline void DrawEnumUI(const char* label, Enum& value) {
        const char* preview = ToString(value).data();

        if (ImGui::BeginCombo(label, preview)) {
            for (auto v : EnumTraits<Enum>::values) {
                bool selected = (v == value);
                if (ImGui::Selectable(ToString(v).data(), selected)) {
                    value = v;
                }
                if (selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
    }

    template <typename T>
    inline void DrawFieldUI(const char* label, std::vector<T>& vec) {
        if (ImGui::TreeNodeEx(label, ImGuiTreeNodeFlags_DefaultOpen)) {
            int remove_index = -1;

            for (size_t i = 0; i < vec.size(); ++i) {
                ImGui::PushID((int)i);

                std::string itemLabel = "Item " + std::to_string(i);
                DrawFieldUI(itemLabel.c_str(), vec[i]);

                if (ImGui::Button("Remove")) {
                    remove_index = (int)i;
                }

                ImGui::Separator();
                ImGui::PopID();
            }

            if (remove_index >= 0) {
                vec.erase(vec.begin() + remove_index);
            }

            if (ImGui::Button("Add")) {
                vec.emplace_back();
            }

            ImGui::TreePop();
        }
    }

    template <typename T>
    inline void DrawVectorUI(const char* label, std::vector<T>& vec) {
        if (!ImGui::TreeNode(label))
            return;

        static int selected = -1;

        // --- List ---
        for (int i = 0; i < (int)vec.size(); ++i) {
            ImGui::PushID(i);

            std::string name = "Item " + std::to_string(i);

            if (ImGui::Selectable(name.c_str(), selected == i)) {
                selected = i;
            }

            ImGui::PopID();
        }

        // --- Controls ---
        if (ImGui::Button("Add")) {
            vec.emplace_back();
        }

        ImGui::SameLine();

        if (selected >= 0 && selected < (int)vec.size()) {
            if (ImGui::Button("Remove")) {
                vec.erase(vec.begin() + selected);
                selected = -1;
            }
        }

        ImGui::Separator();

        // --- Detail Editor ---
        if (selected >= 0 && selected < (int)vec.size()) {
            ImGui::Text("Edit Item %d", selected);
            DrawFieldUI("##edit", vec[selected]);
        }

        ImGui::TreePop();
    }

    inline bool DrawEnumFromRegistry(const char* label, std::string& value,
                                     const std::string& enumName,
                                     EnumRegistry& registry,
                                     bool allowChange = true) {
        auto* values = registry.Get(enumName);
        if (!values) {
            ImGui::Text("Missing enum: %s", enumName.c_str());
            if (allowChange) {
                if (ImGui::Button("Add New Enum?")) {
                    registry.AddValue(enumName, "");
                }
            }
            return false;
        }

        bool changed = false;
        int columns = allowChange ? 2 : 1;
        if (ImGui::BeginCombo(label,
                              value.empty() ? "<select>" : value.c_str())) {
            int delete_index = -1;
            // ========================
            // Existing values
            // ========================
            // for (int i = 0; i < (int)values->size(); ++i) {
            //     auto& v = (*values)[i];
            //     ImGui::PushID(i);
            //     bool selected = (v == value);
            //     if (ImGui::Selectable(v.c_str(), selected)) {
            //         value = v;
            //         changed = true;
            //     }
            //     if (allowChange) {
            //         ImGui::SameLine();
            //         ImGui::SetItemAllowOverlap();
            //         if (ImGui::SmallButton("-")) {
            //             delete_index = i;
            //         }
            //     }
            //     if (selected) {
            //         ImGui::SetItemDefaultFocus();
            //     }
            //     ImGui::PopID();
            // }
            if (ImGui::BeginTable("enum_values", columns,
                                  ImGuiTableFlags_SizingStretchProp)) {
                for (int i = 0; i < (int)values->size(); ++i) {
                    auto& v = (*values)[i];
                    ImGui::PushID(i);
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    bool selected = (v == value);
                    if (ImGui::Selectable(v.c_str(), selected)) {
                        value = v;
                        changed = true;
                    }
                    if (allowChange) {
                        ImGui::TableSetColumnIndex(1);
                        if (ImGui::SmallButton("X")) {
                            delete_index = i;
                        }
                    }
                    ImGui::PopID();
                }
                ImGui::EndTable();
            }
            // ========================
            // Add new value
            // ========================
            if (allowChange) {
                ImGui::Separator();

                static char newValue[128] = "";

                ImGui::InputText("##new_enum_value", newValue,
                                 sizeof(newValue));

                ImGui::SameLine();

                if (ImGui::Button("+")) {
                    std::string v = newValue;

                    if (!v.empty()) {
                        // prevent duplicates
                        auto it = std::find(values->begin(), values->end(), v);

                        if (it == values->end()) {
                            registry.AddValue(enumName, v);
                            value = v;
                            newValue[0] = '\0';
                            changed = true;
                        } else {
                            CORE_WARN("Enum value '{}' already exists", v);
                        }
                    }
                }
            }

            // Delete value
            if (delete_index != -1 && allowChange) {
                if (value == (*values)[delete_index]) {
                    value.clear();  // prevent dangling selection
                }
                registry.EraseAt(enumName, delete_index);
                changed = true;
            }

            ImGui::EndCombo();
        }

        return changed;
    }

    inline void DrawFieldUI(const char* label, FieldRef& f) {
        if (ImGui::TreeNodeEx(label, ImGuiTreeNodeFlags_DefaultOpen)) {
#define X(field) DrawFieldUI(#field, f.field);
            FIELD_REF_FIELDS(X)
#undef X

            ImGui::TreePop();
        }
    }
    inline void DrawFieldUI(const char* label, FieldCondition& c) {
        if (ImGui::TreeNodeEx(label, ImGuiTreeNodeFlags_DefaultOpen)) {
            DrawFieldUI("field", c.field);
            DrawFieldUI("op", c.op);
            DrawFieldUI("value", c.value);
            ImGui::TreePop();
        }
    }

    inline void DrawFieldUI(const char* label, ItemRule& i,
                            bool is_recursive = false) {
        if (ImGui::TreeNodeEx(label, ImGuiTreeNodeFlags_DefaultOpen)) {
            if (!is_recursive) {
                DrawFieldUI("item_name", i.item_name);
                DrawFieldUI("item_short", i.item_short);
                DrawFieldUI("type", i.type);
                DrawFieldUI("presence", i.presence);
                DrawFieldUI("min_length", i.min_length);
                DrawFieldUI("max_length", i.max_length);
            } else {
#define X(field) DrawFieldUI(#field, i.field);
                ITEM_RULE_FIELDS(X)
#undef X
            }
            ImGui::TreePop();
        }
    }
    inline void DrawFieldUI(const char* label, FieldGroupMember& m) {
        if (ImGui::TreeNodeEx(label, ImGuiTreeNodeFlags_DefaultOpen)) {
            DrawFieldUI("record_type", m.record_type);
            DrawFieldUI("field_id", m.field_id);
            ImGui::TreePop();
        }
    }
    inline void DrawFieldUI(const char* label, FieldConditionGroup& g) {
        if (ImGui::TreeNodeEx(label, ImGuiTreeNodeFlags_DefaultOpen)) {
            DrawFieldUI("conditions", g.conditions);
            DrawFieldUI("logic", g.logic);
            ImGui::TreePop();
        }
    }

    inline void DrawFieldUI(const char* label, ValueConstraint& v,
                            EnumRegistry& registry) {
        if (ImGui::TreeNodeEx(label, ImGuiTreeNodeFlags_DefaultOpen)) {
            DrawFieldUI("Type", v.type);
            DrawFieldUI("Op", v.op);
            switch (v.type) {
                case CONSTRAINT_TYPE::NUMERIC:
                    DrawFieldUI("Value", v.numeric_value);
                    break;
                case CONSTRAINT_TYPE::ENUM:
                    DrawFieldUI("Enum", v.enum_name);
                    DrawEnumFromRegistry("Value", v.enum_value, v.enum_name,
                                         registry);
                    break;
                case CONSTRAINT_TYPE::PATTERN:
                    DrawFieldUI("Pattern", v.pattern);
                    break;
                default:
                    break;
            }
            // DrawFieldUI("op", v.op);

            // #define X(field) DrawFieldUI(#field, v.field);
            // VALUE_CONSTRAINT_FIELDS(X)
            // #undef X

            ImGui::TreePop();
        }
    }
    inline void DrawFieldUI(const char* label, ValueConstraint& v) {
        if (ImGui::TreeNodeEx(label, ImGuiTreeNodeFlags_DefaultOpen)) {
#define X(field) DrawFieldUI(#field, v.field);
            VALUE_CONSTRAINT_FIELDS(X)
#undef X

            ImGui::TreePop();
        }
    }
    inline void DrawFieldUI(const char* label, FieldRule& f,
                            bool recursive = false) {
        if (ImGui::TreeNodeEx(label, ImGuiTreeNodeFlags_DefaultOpen)) {
            if (!recursive) {
                DrawFieldUI("field_name", f.field_name);
                DrawFieldUI("field_short", f.field_short);
                ImGui::BeginDisabled();
                DrawFieldUI("record_type", f.record_type);
                ImGui::EndDisabled();
                DrawFieldUI("field_id", f.field_id);
                DrawFieldUI("min_length", f.min_length);
                DrawFieldUI("max_length", f.max_length);
                DrawFieldUI("min_occurs", f.min_occurs);
                DrawFieldUI("max_occurs", f.max_occurs);
            } else {
#define X(field) DrawFieldUI(#field, f.field);
                FIELD_RULE_FIELDS(X)
#undef X
            }
            ImGui::TreePop();
        }
    }
    inline void DrawFieldUI(const char* label, SubfieldRule& s,
                            bool recursive = false) {
        if (ImGui::TreeNodeEx(label, ImGuiTreeNodeFlags_DefaultOpen)) {
            if (!recursive) {
                DrawFieldUI("subfield_name", s.subfield_name);
                DrawFieldUI("subfield_short", s.subfield_short);
                DrawFieldUI("subfield_id", s.subfield_id);
                DrawFieldUI("min_occurs", s.min_occurs);
                DrawFieldUI("max_occurs", s.max_occurs);
            } else {
#define X(field) DrawFieldUI(#field, s.field);
                SUBFIELD_RULE_FIELDS(X)
#undef X
            }
            ImGui::TreePop();
        }
    }
    inline void DrawFieldUI(const char* label, RecordSetRule& r) {
        if (ImGui::TreeNodeEx(label, ImGuiTreeNodeFlags_DefaultOpen)) {
            DrawFieldUI("record_types", r.record_types);
            DrawFieldUI("min_total", r.min_total);
            DrawFieldUI("max_total", r.max_total);

            ImGui::TreePop();
        }
    }
    inline void DrawFieldUI(const char* label, FieldGroupRule& r) {
        if (ImGui::TreeNodeEx(label, ImGuiTreeNodeFlags_DefaultOpen)) {
            DrawFieldUI("type", r.type);
            DrawFieldUI("members", r.members);
            ImGui::TreePop();
        }
    }
    inline void DrawFieldUI(const char* label, LinkRule& r) {
        if (ImGui::TreeNodeEx(label, ImGuiTreeNodeFlags_DefaultOpen)) {
            DrawFieldUI("source", r.source);
            DrawFieldUI("target", r.target);
            DrawFieldUI("must_exist", r.must_exist);

            ImGui::TreePop();
        }
    }
    inline void DrawFieldUI(const char* label, ConditionalConstraint& c) {
        if (ImGui::TreeNodeEx(label, ImGuiTreeNodeFlags_DefaultOpen)) {
            DrawFieldUI("trigger", c.trigger);
            DrawFieldUI("target", c.target);
            DrawFieldUI("enforced", c.enforced);

            ImGui::TreePop();
        }
    }
    inline void DrawFieldUI(const char* label, SemanticRule& s) {
        if (ImGui::TreeNodeEx(label, ImGuiTreeNodeFlags_DefaultOpen)) {
            DrawFieldUI("type", s.type);
            DrawFieldUI("target", s.target);
            DrawFieldUI("parameter", s.parameter);

            ImGui::TreePop();
        }
    }
    inline void DrawFieldUI(const char* label, FieldDependencyRule& r) {
        if (ImGui::TreeNodeEx(label, ImGuiTreeNodeFlags_DefaultOpen)) {
            DrawFieldUI("if_condition", r.if_condition);
            DrawFieldUI("then_op", r.then_op);
            DrawFieldUI("then_field", r.then_field);
            DrawFieldUI("then_value", r.then_value);

            ImGui::TreePop();
        }
    }
    inline void DrawFieldUI(const char* label, RecordRule& r,
                            bool recursive = false) {
        if (ImGui::TreeNodeEx(label, ImGuiTreeNodeFlags_DefaultOpen)) {
            DrawFieldUI("record_name", r.record_name);
            DrawFieldUI("record_short", r.record_short);
            DrawFieldUI("record_type", r.record_type);
            DrawFieldUI("min_occurs", r.min_occurs);
            DrawFieldUI("max_occurs", r.max_occurs);
            if (recursive) {
                DrawFieldUI("applies_when", r.applies_when);
                DrawFieldUI("fields", r.fields);
                DrawFieldUI("dependencies", r.dependencies);
            }
            ImGui::TreePop();
        }
    }

    inline void DrawFieldUI(const char* label, TransactionRule& tx) {
        for (size_t r = 0; r < tx.records.size(); ++r) {
            auto& record = tx.records[r];
            std::string label = "Record: " + std::to_string(r);
            DrawFieldUI(label.c_str(), record);
        }
    }

}  // namespace OpenEFT
