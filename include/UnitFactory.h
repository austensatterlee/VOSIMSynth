#pragma once
#include "Unit.h"
#include <vector>
#include <memory>
#include <set>
#include <unordered_map>
#include <cstdio>

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
			return m_group_names;
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

        int getPrototypeId(const string& a_name) const{
            int i = 0;
            for (shared_ptr<FactoryPrototype> prototype : m_prototypes) {
                if (prototype->name == a_name) {
                    return i;
                }
                i++;
            }
            return -1;
        }

		shared_ptr<Unit> createUnit(int a_protoNum) const {
            shared_ptr<Unit> unit = shared_ptr<Unit>(m_prototypes[a_protoNum]->prototype->clone());
			char namebuf[MAX_UNIT_STR_LEN];
			snprintf(namebuf, 256, "%s_%d", unit->getName().c_str(), m_prototypes[a_protoNum]->build_count);
			string newname = namebuf;
			unit->_setName(newname);
			m_prototypes[a_protoNum]->build_count++;
			return unit;
		}

        shared_ptr<Unit> createUnit(string a_name) {
			int i = 0;
			for (shared_ptr<FactoryPrototype> prototype : m_prototypes) {
				if (prototype->name == a_name) {
					return createUnit(i);
				}
				i++;
			}
			throw std::domain_error("Unit name not found in factory.");
		}

        shared_ptr<Unit> createUnit(unsigned int a_classIdentifier) const {
			int protonum = m_class_identifiers.at(a_classIdentifier);
			return createUnit(protonum);
		}

		bool hasClassId(unsigned a_classIdentifier) const {
			bool result = m_class_identifiers.find(a_classIdentifier) != m_class_identifiers.end();
			return result;
		}

	protected:
		vector<shared_ptr<FactoryPrototype>> m_prototypes;
		set<string> m_group_names;

		unordered_map<unsigned int, int> m_class_identifiers; //<! mapping from unique class IDs to prototype numbers
	};
}

