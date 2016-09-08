#include "UnitFactory.h"
#include "Containers.h"
#include "Unit.h"

syn::FactoryPrototype::FactoryPrototype(std::string a_group_name, Unit* a_unit, size_t a_class_size)
	: classIdentifier(a_unit->getClassIdentifier()),
	  group_name(a_group_name),
	  name(a_unit->name()),
	  prototype(a_unit),
	  build_count{0},
	  size(a_class_size) 
{}

set<string> syn::UnitFactory::getGroupNames() const {
	set<string> groupnames;
	for (const string& groupname : m_group_names) {
		if (groupname != "")
			groupnames.insert(groupname);
	}
	return groupnames;
}

vector<string> syn::UnitFactory::getPrototypeNames(const string& group) const {
	vector<string> names;
	for (const FactoryPrototype& prototype : m_prototypes) {
		if (prototype.group_name == group) {
			names.push_back(prototype.name);
		}
	}
	return names;
}

vector<string> syn::UnitFactory::getPrototypeNames() const {
	vector<string> names;
	for (const FactoryPrototype& prototype : m_prototypes) {
		names.push_back(prototype.name);
	}
	return names;
}

const syn::FactoryPrototype* syn::UnitFactory::getFactoryPrototype(const string& a_prototypeName) const {
	for (const FactoryPrototype& prototype : m_prototypes) {
		if (prototype.name == a_prototypeName)
			return &prototype;
	}
	return nullptr;
}

syn::Unit* syn::UnitFactory::createUnit(int a_protoNum, const string& a_name) {
	Unit* unit = m_prototypes[a_protoNum].prototype->clone();
	// Generate default name
	unit->_setName(m_prototypes[a_protoNum].name);
	return unit;
}

syn::Unit* syn::UnitFactory::createUnit(unsigned a_classIdentifier, const string& a_name) {
	int protonum = getPrototypeIdx_(a_classIdentifier);
	if (protonum < 0)
		return nullptr;
	return createUnit(protonum, a_name);
}

syn::Unit* syn::UnitFactory::createUnit(string a_prototypeName, const string& a_name) {
	int protonum = getPrototypeIdx_(a_prototypeName);
	if (protonum < 0)
		return nullptr;
	return createUnit(protonum, a_name);
}

bool syn::UnitFactory::hasClassId(unsigned a_classIdentifier) const {
	bool result = m_class_identifiers.find(a_classIdentifier) != m_class_identifiers.end();
	return result;
}

bool syn::UnitFactory::hasClassId(string a_protoName) const {
	int i = 0;
	for (const FactoryPrototype& prototype : m_prototypes) {
		if (prototype.name == a_protoName) {
			return true;
		}
		i++;
	}
	return false;
}

unsigned syn::UnitFactory::getClassId(string a_protoName) const {
	return m_prototypes[getPrototypeIdx_(a_protoName)].prototype->getClassIdentifier();
}

unsigned syn::UnitFactory::getClassId(int a_protoNum) const {
	return m_prototypes[a_protoNum].prototype->getClassIdentifier();
}

unsigned syn::UnitFactory::getClassId(unsigned a_protoNum) {
	return a_protoNum;
}

void syn::UnitFactory::resetBuildCounts() {
	for (FactoryPrototype& prototype : m_prototypes) {
		prototype.build_count = 0;
	}
}

int syn::UnitFactory::getPrototypeIdx_(const string& a_name) const {
	int i = 0;
	for (const FactoryPrototype& prototype : m_prototypes) {
		if (prototype.name == a_name) {
			return i;
		}
		i++;
	}
	return -1;
}

int syn::UnitFactory::getPrototypeIdx_(unsigned a_classId) const {
	if (m_class_identifiers.find(a_classId) != m_class_identifiers.end()) {
		return m_class_identifiers.at(a_classId);
	}
	return -1;
}
