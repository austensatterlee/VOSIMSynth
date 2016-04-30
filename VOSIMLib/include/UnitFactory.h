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
#include <Containers.h>

#define MAX_UNIT_STR_LEN 256

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
            build_count{0}
        {};

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
        void addUnitPrototype(string a_group_name, shared_ptr<Unit> a_unit) {
            shared_ptr<FactoryPrototype> prototype = shared_ptr<FactoryPrototype> (new FactoryPrototype(a_group_name, a_unit));
            m_prototypes.push_back(prototype);
            m_group_names.insert(prototype->group_name);
            m_class_identifiers[prototype->prototype->getClassIdentifier()] = m_prototypes.size() - 1;
        }

        void addUnitPrototype(string a_group_name, Unit* a_unit) {
            addUnitPrototype(a_group_name, shared_ptr<Unit>(a_unit));
        }

        set<string> getGroupNames() const {
			set<string> groupnames;
			for(string groupname:  m_group_names) {
				if (groupname != "")
					groupnames.insert(groupname);
			}
            return groupnames;
        }

        vector<string> getPrototypeNames(const string& group) const {
            vector<string> names;
            for (shared_ptr<FactoryPrototype> prototype : m_prototypes) {
                if (prototype->group_name == group) {
                    names.push_back(prototype->name);
                }
            }
            return names;
        }

        Unit* createUnit(int a_protoNum) const {
            Unit* unit = m_prototypes[a_protoNum]->prototype->clone();
            char namebuf[MAX_UNIT_STR_LEN];
            snprintf(namebuf, 256, "%s_%d", unit->getName().c_str(), m_prototypes[a_protoNum]->build_count);
            string newname = namebuf;
            unit->_setName(newname);
            m_prototypes[a_protoNum]->build_count++;
            return unit;
        }

        Unit* createUnit(unsigned a_classIdentifier) const {
            int protonum = getPrototypeIdx_(a_classIdentifier);
            return createUnit(protonum);
        }

		Unit* createUnit(string a_prototypeName) const {
			int protonum = getPrototypeIdx_(a_prototypeName);
			return createUnit(protonum);
		}

        bool hasClassId(unsigned a_classIdentifier) const {
            bool result = m_class_identifiers.find(a_classIdentifier) != m_class_identifiers.end();
            return result;
        }

		bool hasClassId(string a_protoName) const {
			int i = 0;
			for (shared_ptr<FactoryPrototype> prototype : m_prototypes) {
				if (prototype->name == a_protoName) {
					return true;
				}
				i++;
			}
			return false;
		}

		unsigned getClassId(string a_protoName) const {
			return m_prototypes[getPrototypeIdx_(a_protoName)]->prototype->getClassIdentifier();
        }

		unsigned getClassId(int a_protoNum) const {
			return m_prototypes[a_protoNum]->prototype->getClassIdentifier();
        }

	    static unsigned getClassId(unsigned a_protoNum) {
            return a_protoNum;
        }

		void resetBuildCounts() {
            for (shared_ptr<FactoryPrototype> prototype : m_prototypes) {
				prototype->build_count = 0;
            }
        }

	    static void saveUnit(const Unit* a_unit, int a_unitId, ByteChunk* a_data) {
			unsigned classId = a_unit->getClassIdentifier();
			a_data->Put<unsigned>(&classId);
			a_data->Put<int>(&a_unitId);
			a_data->PutStr(a_unit->getName().c_str());
			int nParams = a_unit->getNumParameters();
			a_data->Put<int>(&nParams);
			for(int i=0;i<nParams;i++) {
				double val = a_unit->getParameter(i).getDouble();
				int prec = a_unit->getParameter(i).getPrecision();
				a_data->Put<double>(&val);
				a_data->Put<int>(&prec);
			}
        }

		int loadUnit(ByteChunk* a_data, int a_startPos, Unit** a_unit, int* a_unitId) const {
			int startPos = a_startPos;

			unsigned classId;
			startPos = a_data->Get<unsigned>(&classId, startPos);

			int unitId;
			startPos = a_data->Get<int>(&unitId, startPos);

			WDL_String tmp_unitName;
			startPos = a_data->GetStr(&tmp_unitName, startPos);
			string unitName = tmp_unitName.Get();

			int nParams;
			startPos = a_data->Get<int>(&nParams, startPos);

			*a_unit = createUnit(classId);
			(*a_unit)->_setName(unitName);
			*a_unitId = unitId;
			for(int i=0;i<nParams;i++) {
				double val;
				int prec;
				startPos = a_data->Get<double>(&val, startPos);
				startPos = a_data->Get<int>(&prec, startPos);
				(*a_unit)->getParameter_(i).set(val);
				(*a_unit)->getParameter_(i).setPrecision(prec);
			}
			for(int i=0;i<nParams;i++) {
				(*a_unit)->onParamChange_(i);
			}
			return startPos;
        }
    protected:
		int getPrototypeIdx_(const string& a_name) const {
			int i = 0;
			for (shared_ptr<FactoryPrototype> prototype : m_prototypes) {
				if (prototype->name == a_name) {
					return i;
				}
				i++;
			}
			return -1;
		}


		int getPrototypeIdx_(unsigned a_classId) const {
			if (m_class_identifiers.find(a_classId) != m_class_identifiers.end()) {
				return m_class_identifiers.at(a_classId);
			}
			return -1;
		}
	private:
        vector<shared_ptr<FactoryPrototype>> m_prototypes;
        set<string> m_group_names;

        unordered_map<unsigned int, int> m_class_identifiers; //<! mapping from unique class IDs to prototype numbers
    };
}

#endif