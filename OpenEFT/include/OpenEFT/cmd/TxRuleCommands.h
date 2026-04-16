// Copyright (c) 2026 Noah G. Wood
// SPDX-License-Identifier: LicenseRef-OpenEFT
//
// This file is part of OpenEFT.
// OpenEFT is source-available software. See LICENSE file for details.
#pragma once

#include <NMTK/cmd/ICommand.h>
#include <OpenEFT/core/Project.h>

#include <filesystem>
#include <optional>
#include <string>

namespace OpenEFT {
    class CreateTransactionCommand : public NMTK::ICommand {
      public:
        CreateTransactionCommand(std::string name)
            : m_Name(std::move(name)), m_Rule({}) {}
        CreateTransactionCommand(TransactionRule& txr) : m_Rule(txr) {
            m_Name = txr.name;
        }

        const std::string Name() const override { return "Create Transaction"; }

        void Execute(NMTK::IProject& project) override {
            auto& proj = static_cast<Project&>(project);
            auto* profile = proj.Profiles().GetActive();
            if (!profile)
                return;

            auto& txs = profile->AllTransactions();

            if (txs.contains(m_Name)) {
                CORE_WARN("Transaction {} already exists", m_Name);
                return;
            }

            txs[m_Name] = m_Rule;
            txs[m_Name].name = m_Name;
        }

        void Undo(NMTK::IProject& project) override {
            auto& proj = static_cast<Project&>(project);
            auto* profile = proj.Profiles().GetActive();
            if (!profile)
                return;

            profile->AllTransactions().erase(m_Name);
        }

      private:
        std::string m_Name;
        TransactionRule m_Rule;
    };
    class DeleteTransactionCommand : public NMTK::ICommand {
      public:
        DeleteTransactionCommand(std::string name) : m_Name(std::move(name)) {}

        const std::string Name() const override { return "Delete Transaction"; }

        void Execute(NMTK::IProject& project) override {
            auto& proj = static_cast<Project&>(project);
            auto* profile = proj.Profiles().GetActive();
            if (!profile)
                return;

            auto& txs = profile->AllTransactions();

            auto it = txs.find(m_Name);
            if (it == txs.end())
                return;

            m_Backup = it->second;  // copy for undo
            txs.erase(it);

            // fix active selection
            if (proj.Profiles().IsActiveTransaction(m_Name)) {
                proj.Profiles().SetActiveTransaction("");
            }
        }

        void Undo(NMTK::IProject& project) override {
            if (!m_Backup.has_value())
                return;

            auto& proj = static_cast<Project&>(project);
            auto* profile = proj.Profiles().GetActive();
            if (!profile)
                return;

            profile->AllTransactions()[m_Backup->name] = *m_Backup;
        }

      private:
        std::string m_Name;
        std::optional<TransactionRule> m_Backup;
    };

    class RenameTransactionCommand : public NMTK::ICommand {
      public:
        RenameTransactionCommand(std::string oldName, std::string newName)
            : m_Old(std::move(oldName)), m_New(std::move(newName)) {}

        const std::string Name() const override { return "Rename Transaction"; }

        void Execute(NMTK::IProject& project) override {
            auto& proj = static_cast<Project&>(project);
            auto* profile = proj.Profiles().GetActive();
            if (!profile)
                return;

            auto& txs = profile->AllTransactions();

            auto it = txs.find(m_Old);
            if (it == txs.end())
                return;

            auto node = txs.extract(it);
            node.key() = m_New;
            node.mapped().name = m_New;

            txs.insert(std::move(node));

            if (proj.Profiles().IsActiveTransaction(m_Old)) {
                proj.Profiles().SetActiveTransaction(m_New);
            }
        }

        void Undo(NMTK::IProject& project) override {
            auto& proj = static_cast<Project&>(project);
            auto* profile = proj.Profiles().GetActive();
            if (!profile)
                return;

            auto& txs = profile->AllTransactions();

            auto it = txs.find(m_New);
            if (it == txs.end())
                return;

            auto node = txs.extract(it);
            node.key() = m_Old;
            node.mapped().name = m_Old;

            txs.insert(std::move(node));

            if (proj.Profiles().IsActiveTransaction(m_New)) {
                proj.Profiles().SetActiveTransaction(m_Old);
            }
        }

      private:
        std::string m_Old, m_New;
    };

    class SetActiveTransactionCommand : public NMTK::ICommand {
      public:
        SetActiveTransactionCommand(std::string name)
            : m_New(std::move(name)) {}

        const std::string Name() const override {
            return "Set Active Transaction";
        }

        void Execute(NMTK::IProject& project) override {
            auto& proj = static_cast<Project&>(project);

            m_Previous = proj.Profiles().GetActiveTransactionName();
            proj.Profiles().SetActiveTransaction(m_New);
        }

        void Undo(NMTK::IProject& project) override {
            auto& proj = static_cast<Project&>(project);
            proj.Profiles().SetActiveTransaction(m_Previous);
        }

      private:
        std::string m_New;
        std::string m_Previous;
    };
}  // namespace OpenEFT
