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
#include <memory>
#include <set>
#include <unordered_map>

#define MAX_UNIT_NAME_LEN 256

class ByteChunk;

using std::vector;
using std::set;
using std::unordered_map;
using std::shared_ptr;

namespace syn
{
	struct FactoryPrototype
	{
		FactoryPrototype(string a_group_name, shared_ptr<Unit> a_unit) :
			group_name(a_group_name),
			name(a_unit.get()->getName()),
			prototype(a_unit),
			build_count{0} {};

		string group_name;
		string name;
		shared_ptr<Unit> prototype;
		int build_count;
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
		void addUnitPrototype(string a_group_name, shared_ptr<Unit> a_unit);

		void addUnitPrototype(string a_group_name, Unit* a_unit);

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
		vector<shared_ptr<FactoryPrototype>> m_prototypes;
		set<string> m_group_names;
		set<string> m_generatedNameHistory;

		unordered_map<unsigned int, int> m_class_identifiers; //<! mapping from unique class IDs to prototype numbers
	};
}

#endif
