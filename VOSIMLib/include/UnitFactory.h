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

#include "Unit.h"

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
	struct FactoryPrototype
	{
		FactoryPrototype(string a_group_name, Unit* a_unit, size_t a_class_size) :
			group_name(a_group_name),
			name(a_unit->getName()),
			prototype(a_unit),
			build_count{0},
			size(a_class_size)
		{}

		string group_name;
		string name;
		Unit* prototype;
		int build_count;
		size_t size;
	};

	class UnitFactory
	{
	public:

		UnitFactory() {}

		virtual ~UnitFactory() {
			m_prototypes.clear();
			m_group_names.clear();
		}

		/**
		 * \brief Register a prototype unit with the factory. Prototype deletion will be taken care of upon factory destruction.
		 */
		template <typename T, typename = std::enable_if_t<std::is_base_of<Unit, T>::value > >
		void addUnitPrototype(const string& a_group_name, const string& a_unit_name);

		set<string> getGroupNames() const;

		vector<string> getPrototypeNames(const string& group) const;

		Unit* createUnit(int a_protoNum, const string& a_name = "");

		Unit* createUnit(unsigned a_classIdentifier, const string& a_name = "");

		Unit* createUnit(string a_prototypeName, const string& a_name = "");

		bool hasClassId(unsigned a_classIdentifier) const;

		bool hasClassId(string a_protoName) const;

		unsigned getClassId(string a_protoName) const;

		unsigned getClassId(int a_protoNum) const;

		static unsigned getClassId(unsigned a_protoNum);

		void resetBuildCounts();

		static void saveUnit(const Unit* a_unit, int a_unitId, ByteChunk* a_data);

		int loadUnit(ByteChunk* a_data, int a_startPos, Unit** a_unit, int* a_unitId);

	protected:
		int getPrototypeIdx_(const string& a_name) const;


		int getPrototypeIdx_(unsigned a_classId) const;

	private:
		vector<FactoryPrototype*> m_prototypes;
		set<string> m_group_names;
		set<string> m_generatedNameHistory;

		unordered_map<unsigned int, int> m_class_identifiers; //<! mapping from unique class IDs to prototype numbers
	};

	template <typename T, typename>
	void UnitFactory::addUnitPrototype(const string& a_group_name, const string& a_unit_name) {
		FactoryPrototype* prototype = new FactoryPrototype(a_group_name, new T(a_unit_name), sizeof(T));
		m_prototypes.push_back(prototype);
		m_group_names.insert(prototype->group_name);
		m_class_identifiers[prototype->prototype->getClassIdentifier()] = m_prototypes.size() - 1;
	}
}

#endif
