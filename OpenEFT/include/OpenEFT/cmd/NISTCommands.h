#pragma once
#include <NMTK/cmd/ICommand.h>
#include <OpenEFT/core/Project.h>
#include <nnist/nnist.h>

#include <filesystem>

namespace OpenEFT {

    // Use this to avoid remapping destroying pointers in mutation operations
    struct NodeRef {
        std::filesystem::path file;

        int record = -1;
        int field = -1;
        int subfield = -1;
        int item = -1;
    };

    template <typename Parent>
    struct ContainerTraits;

    template <>
    struct ContainerTraits<nnist::File> {
        static auto& Get(nnist::File& f) { return f.records; }
    };

    template <>
    struct ContainerTraits<nnist::Record> {
        static auto& Get(nnist::Record& r) { return r.fields; }
    };

    template <>
    struct ContainerTraits<nnist::Field> {
        static auto& Get(nnist::Field& f) { return f.subfields; }
    };

    template <>
    struct ContainerTraits<nnist::Subfield> {
        static auto& Get(nnist::Subfield& s) { return s.items; }
    };

    template <typename Parent, typename T>
    class NISTAddCommand : public NMTK::ICommand {
      public:
        NISTAddCommand(NodeRef parent, T value)
            : m_Parent(parent), m_Value(std::move(value)) {}

        void Execute(NMTK::IProject& project) override {
            Parent* parent = Resolve<Parent>(project, m_Parent);
            if (!parent)
                return;

            auto& container = ContainerTraits<Parent>::Get(*parent);

            container.push_back(m_Value);
            m_Index = container.size() - 1;
            m_Executed = true;
        }

        void Undo(NMTK::IProject& project) override {
            Parent* parent = Resolve<Parent>(project, m_Parent);
            if (!parent || !m_Executed)
                return;

            auto& container = ContainerTraits<Parent>::Get(*parent);

            if (m_Index < container.size())
                container.erase(container.begin() + m_Index);
        }

      private:
        NodeRef m_Parent;
        T m_Value;
        size_t m_Index = 0;
        bool m_Executed = false;
    };

    template <typename Parent, typename T>
    class NISTDeleteCommand : public NMTK::ICommand {
      public:
        NISTDeleteCommand(NodeRef parent, size_t index)
            : m_Parent(parent), m_Index(index) {}

        void Execute(NMTK::IProject& project) override {
            Parent* parent = Resolve<Parent>(project, m_Parent);
            if (!parent)
                return;

            auto& container = ContainerTraits<Parent>::Get(*parent);

            if (m_Index >= container.size())
                return;

            m_Deleted = container[m_Index];
            container.erase(container.begin() + m_Index);
            m_Executed = true;
        }

        void Undo(NMTK::IProject& project) override {
            Parent* parent = Resolve<Parent>(project, m_Parent);
            if (!parent || !m_Executed)
                return;

            auto& container = ContainerTraits<Parent>::Get(*parent);

            if (m_Index <= container.size())
                container.insert(container.begin() + m_Index, m_Deleted);
        }

      private:
        NodeRef m_Parent;
        size_t m_Index;

        T m_Deleted;
        bool m_Executed = false;
    };

    template <typename T>
    class NISTMutateCommand : public NMTK::ICommand {
      public:
        using Mutator = std::function<void(T&)>;

        NISTMutateCommand(NodeRef ref, Mutator fn) : m_Ref(ref), m_Mutator(fn) {}

        void Execute(NMTK::IProject& project) override {
            T* target = Resolve<T>(project, m_Ref);
            if (!target)
                return;

            m_Before = *target;
            m_Mutator(*target);
            m_Executed = true;
        }

        void Undo(NMTK::IProject& project) override {
            T* target = Resolve<T>(project, m_Ref);
            if (!target || !m_Executed)
                return;

            *target = m_Before;
        }

      private:
        NodeRef m_Ref;
        Mutator m_Mutator;

        T m_Before;
        bool m_Executed = false;
    };
}  // namespace OpenEFT

/**
    * @brief NNIST File Command Usage:
    *
    * Adding:
   CommandStack.Execute(
       NISTAddCommand<nnist::Record, nnist::Field>(
           selected_record_ref,
           nnist::Field{}
       )
   );
    * Deletion:
   CommandStack.Execute(
       NISTDeleteCommand<nnist::Subfield, nnist::Item>(
           selected_subfield_ref,
           item_index
       )
   );
    * Mutation
    CommandStack.Execute(
       NISTMutateCommand<nnist::Field>(
           field_ref,
           [](auto& f) {
               f.subfields.clear();
           }
       )
   );
    */