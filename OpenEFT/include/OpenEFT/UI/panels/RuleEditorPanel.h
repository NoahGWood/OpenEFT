// Copyright (c) 2026 Noah G. Wood
// SPDX-License-Identifier: LicenseRef-OpenEFT
//
// This file is part of OpenEFT.
// OpenEFT is source-available software. See LICENSE file for details.
#pragma once
#include <NMTK/NMTK.h>
#include <OpenEFT/UI/widgets/Widgets.h>
#include <OpenEFT/cmd/Commands.h>
#include <OpenEFT/rules/EnumRegistry.h>
#include <OpenEFT/rules/Rules.h>
#include <OpenEFT/rules/RulesUI.h>
#include <math.h>
namespace OpenEFT {

    // Testing code, not really needed anymore just leaving for now
    // inline static std::vector<TransactionRule> transactions = {
    //     {"Federal Applicant User Fee", "FAUF", "CJIS"},
    //     {"Criminal Tenprint Submission", "CAR", "CJIS"}};

    // inline static EnumRegistry registry;

    struct RuleEditorPanel : NMTK::IPanel {
        RuleEditorPanel(std::shared_ptr<NMTK::AppContext> ctx)
            : NMTK::IPanel(std::move(ctx)) {
            m_Name = "Rule Editor";
            m_IsOpen = false;
        }
        Profile* active_profile = nullptr;
        MapEditor<TransactionRule> tx_editor;
        ListEditor<RecordRule> record_editor;
        ListEditor<FieldRule> field_editor;
        ListEditor<SubfieldRule> subfield_editor;
        ListEditor<ItemRule> item_editor;
        ListEditor<ValueConstraint> constraint_editor;

        TransactionRule* selected_tx = nullptr;
        RecordRule* selected_record = nullptr;
        FieldRule* selected_field = nullptr;
        SubfieldRule* selected_subfield = nullptr;
        ItemRule* selected_item = nullptr;
        ValueConstraint* selected_constraint = nullptr;

        void ClearFromLevel(int level) {
            switch (level) {
                case 0:
                    selected_tx = nullptr;
                    tx_editor.selected = nullptr;
                case 1:
                    selected_record = nullptr;
                    record_editor.selected = nullptr;
                case 2:
                    selected_field = nullptr;
                    field_editor.selected = nullptr;
                case 3:
                    selected_subfield = nullptr;
                    subfield_editor.selected = nullptr;
                case 4:
                    selected_item = nullptr;
                    item_editor.selected = nullptr;
                case 5:
                    selected_constraint = nullptr;
                    constraint_editor.selected = nullptr;
                default:
                    break;
            }
        }

        void RenderTX() {
            auto& tx_map = active_profile->AllTransactions();
            tx_editor.Draw(
                &tx_map, "EditTx", "AddTx", "+ Add Transaction",
                // --- label ---
                [](const std::string& label, const TransactionRule& tx) {
                    return label.empty()
                               ? tx.name.empty() ? tx.transaction_type : tx.name
                               : label;
                },
                // --- edit popup ---
                [this](TransactionRule& tx) {
                    std::string name = tx.name;
                    DrawFieldUI("Edit Transaction", tx);
                    if (tx.name !=
                        name) {  // Make sure we sync the name with the mapping
                        m_AppContext->cmdManager().Run(
                            std::make_unique<RenameTransactionCommand>(name,
                                                                       tx.name),
                            m_AppContext->project());
                    }
                },
                // --- add popup ---
                [](TransactionRule& tx) { DrawFieldUI("New Transaction", tx); },
                // --- on add ---
                [this](const std::string& name, TransactionRule& tx) {
                    m_AppContext->cmdManager().Run(
                        std::make_unique<CreateTransactionCommand>(tx),
                        m_AppContext->project());
                },
                // --- on select ---
                [this]() {
                    ClearFromLevel(1);
                    m_AppContext->cmdManager().Run(
                        std::make_unique<SetActiveTransactionCommand>(
                            tx_editor.selected->name),
                        m_AppContext->project());
                },
                [this]() {
                    m_AppContext->cmdManager().Run(
                        std::make_unique<DeleteTransactionCommand>(
                            tx_editor.selected->name),
                        m_AppContext->project());
                    ClearFromLevel(0);
                });
        }

        void RenderRecords() {
            record_editor.Draw(
                selected_tx ? &selected_tx->records : nullptr, "EditRecord",
                "AddRecord", "+ Add Record",
                // Label
                [](const RecordRule& r) {
                    return r.record_short.empty() ? r.record_name
                                                  : r.record_short;
                },
                // Edit
                [](RecordRule& r) { DrawFieldUI("Edit Record", r); },
                // Add
                [](RecordRule& r) { DrawFieldUI("New Record", r); },
                // On Add
                [&](RecordRule& r) {
                    selected_tx->records.push_back(r);
                    selected_record = nullptr;
                },
                // On Select
                [&]() {
                    ClearFromLevel(2);
                    selected_record = record_editor.selected;
                },
                [&]() { ClearFromLevel(1); });

            selected_record = record_editor.selected;
        }
        void RenderFields() {
            field_editor.Draw(
                selected_record ? &selected_record->fields : nullptr,
                "EditField", "AddField", "+ Add Field",
                [](const FieldRule& f) {
                    return f.field_short.empty() ? f.field_name : f.field_short;
                },
                [](FieldRule& f) { DrawFieldUI("Edit Field", f); },
                [](FieldRule& f) { DrawFieldUI("New Field", f); },
                [&](FieldRule& f) { selected_record->fields.push_back(f); },
                [&]() {
                    ClearFromLevel(3);
                    selected_field = field_editor.selected;
                },
                [&]() { ClearFromLevel(2); });
            selected_field = field_editor.selected;
        }
        void RenderSubfields() {
            subfield_editor.Draw(
                selected_field ? &selected_field->subfields : nullptr,
                "EditSubField", "AddSubField", "+ Add SubField",
                [](const SubfieldRule& f) {
                    return f.subfield_short.empty() ? f.subfield_name
                                                    : f.subfield_short;
                },
                [](SubfieldRule& f) { DrawFieldUI("Edit Subfield", f); },
                [](SubfieldRule& f) { DrawFieldUI("New Subfield", f); },
                [&](SubfieldRule& f) {
                    selected_field->subfields.push_back(f);
                },
                [&]() {
                    ClearFromLevel(4);
                    selected_subfield = subfield_editor.selected;
                },
                [&]() { ClearFromLevel(3); });
            selected_subfield = subfield_editor.selected;
        }
        void RenderItems() {
            item_editor.Draw(
                selected_subfield ? &selected_subfield->items : nullptr,
                "EditItem", "AddItem", "+ Add Item",
                [](const ItemRule& i) {
                    return i.item_short.empty() ? i.item_name : i.item_short;
                },
                [](ItemRule& i) { DrawFieldUI("Edit Subfield", i); },
                [](ItemRule& i) { DrawFieldUI("New Subfield", i); },
                [&](ItemRule& i) {
                    selected_subfield->items.push_back(i);
                    selected_item = nullptr;
                },
                [&]() {
                    ClearFromLevel(5);
                    selected_item = item_editor.selected;
                },
                [&]() { ClearFromLevel(4); });
            selected_item = item_editor.selected;
        }
        void RenderConstraints() {
            constraint_editor.Draw(
                selected_item ? &selected_item->constraints : nullptr,
                "EditConstraint", "AddConstraint", "+ Add Constraint",
                [](const ValueConstraint& c) { return "Constraint-"; },
                [&](ValueConstraint& c) {
                    DrawFieldUI("Edit Constraint", c, active_profile->Enums());
                },
                [&](ValueConstraint& c) {
                    DrawFieldUI("New Constraint", c, active_profile->Enums());
                },
                [&](ValueConstraint& c) {
                    selected_item->constraints.push_back(c);
                    selected_constraint = nullptr;
                },
                [&]() { selected_constraint = constraint_editor.selected; },
                [&]() { ClearFromLevel(5); }, true);
            selected_constraint = constraint_editor.selected;
        }
        void Render() override {
            ImGui::Begin(m_Name.c_str(), &m_IsOpen);
            auto& project = static_cast<Project&>(m_AppContext->project());
            active_profile = project.Profiles().GetActive();
            if (active_profile == nullptr) {
                ImGui::TextDisabled(
                    "No profile selected. Do so in profile menu.");
                ImGui::End();
                return;
            }
            selected_tx = project.Profiles().GetActiveTransaction();

            if (ImGui::BeginTable("Rules Editor", 6)) {
                ImGui::TableSetupColumn("Transactions");
                ImGui::TableSetupColumn("Records");
                ImGui::TableSetupColumn("Fields");
                ImGui::TableSetupColumn("Subfields");
                ImGui::TableSetupColumn("Items");
                ImGui::TableSetupColumn("Constraints");
                ImGui::TableHeadersRow();
                ImGui::TableNextColumn();
                RenderTX();
                ImGui::TableNextColumn();
                RenderRecords();
                ImGui::TableNextColumn();
                RenderFields();
                ImGui::TableNextColumn();
                RenderSubfields();
                ImGui::TableNextColumn();
                RenderItems();
                ImGui::TableNextColumn();
                RenderConstraints();
                ImGui::EndTable();
            }
            ImGui::End();
        }
    };
}  // namespace OpenEFT
