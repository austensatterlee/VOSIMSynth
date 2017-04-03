/*
Copyright 2016, Austen Satterlee

This file is part of VOSIMProject.

VOSIMProject is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

VOSIMProject is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with VOSIMProject. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __UNITFACTORY__
#define __UNITFACTORY__
#include "common.h"
#include <vector>
#include <set>
#include <unordered_map>

#define MAX_UNIT_NAME_LEN 256

class ByteChunk;

using std::vector;
using std::set;
using std::unordered_map;

namespace syn
{
    class Unit;

    class VOSIMLIB_API UnitFactory
    {
        UnitFactory() {} 
        
        struct FactoryPrototype
        {
            FactoryPrototype(std::string a_group_name, Unit* a_unit, size_t a_class_size);

            unsigned int classIdentifier;
            std::string group_name;
            std::string name;
            Unit* prototype;
            int build_count;
            size_t size;
        };

    public:

        UnitFactory(const UnitFactory&) = delete;
        void operator=(const UnitFactory&) = delete;

        static UnitFactory& instance() {
            static UnitFactory singleton;
            return singleton;
        }

        virtual ~UnitFactory();

        /**
         * \brief Register a prototype unit with the factory. Prototype deletion will be taken care of upon factory destruction.
         */
        template <typename T, typename = std::enable_if_t<std::is_base_of<Unit, T>::value>>
        void addUnitPrototype(const std::string& a_group_name, const std::string& a_unit_name);

        set<std::string> getGroupNames() const;

        vector<std::string> getPrototypeNames(const std::string& group) const;

        vector<std::string> getPrototypeNames() const;

        const std::string& getPrototypeName(unsigned a_classIdentifier);

        const FactoryPrototype* getFactoryPrototype(const std::string& a_prototypeName) const;

        const FactoryPrototype* getFactoryPrototype(const Unit& a_unit) const;

        Unit* createUnit(unsigned a_classIdentifier, const std::string& a_name = "");

        Unit* createUnit(std::string a_prototypeName, const std::string& a_name = "");

        std::string generateUnitName(Unit* a_unit) const;

        bool hasClassId(unsigned a_classIdentifier) const;

        bool hasClassId(std::string a_protoName) const;

        unsigned getClassId(std::string a_protoName) const;

        unsigned getClassId(int a_protoNum) const;

        static unsigned getClassId(unsigned a_protoNum);

        void resetBuildCounts();

    protected:
        int getPrototypeIdx_(const std::string& a_name) const;

        int getPrototypeIdx_(unsigned a_classId) const;

        Unit* createUnit_(int a_protoNum, const std::string& a_name);

    private:
        vector<FactoryPrototype> m_prototypes;
        set<std::string> m_group_names;

        unordered_map<unsigned int, int> m_class_identifiers; //<! mapping from unique class IDs to prototype numbers
    };

    template <typename T, typename>
    void UnitFactory::addUnitPrototype(const std::string& a_group_name, const std::string& a_unit_name) {
        FactoryPrototype prototype{a_group_name, new T(a_unit_name), sizeof(T)};
        if (m_class_identifiers.find(prototype.classIdentifier) != m_class_identifiers.end())
            return;
        m_prototypes.push_back(prototype);
        m_group_names.insert(prototype.group_name);
        m_class_identifiers[prototype.classIdentifier] = m_prototypes.size() - 1;
    }
}

#endif
