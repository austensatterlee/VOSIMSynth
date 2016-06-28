#include "UnitFactory.h"
#include "Containers.h"

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
	string newname;
	if (!a_name.empty() && m_generatedNameHistory.find(a_name) == m_generatedNameHistory.end()) {
		newname = a_name;
	}
	else {
		char namebuf[MAX_UNIT_NAME_LEN];
		do {
			snprintf(namebuf, 256, "%s_%d", unit->getName().c_str(), m_prototypes[a_protoNum].build_count);
			newname = namebuf;
			m_prototypes[a_protoNum].build_count++;
		} while (m_generatedNameHistory.find(newname) != m_generatedNameHistory.end());
	}

	unit->_setName(newname);
	m_generatedNameHistory.insert(newname);
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
	m_generatedNameHistory.clear();
}

void syn::UnitFactory::saveUnit(const Unit* a_unit, int a_unitId, ByteChunk* a_data) {
	unsigned classId = a_unit->getClassIdentifier();
	a_data->Put<unsigned>(&classId);
	a_data->Put<int>(&a_unitId);
	a_data->PutStr(a_unit->getName().c_str());
	int nParams = a_unit->getNumParameters();
	a_data->Put<int>(&nParams);
	for (int i = 0; i < nParams; i++) {
		double val = a_unit->getParameter(i).getDouble();
		int prec = a_unit->getParameter(i).getPrecision();
		a_data->Put<double>(&val);
		a_data->Put<int>(&prec);
	}
	int reserved = 64;
	a_data->Put<int>(&reserved);
	for (int i = 0; i < reserved; i++) {
		char reservedByte = 0;
		a_data->Put<char>(&reservedByte);
	}
}

int syn::UnitFactory::loadUnit(ByteChunk* a_data, int a_startPos, Unit** a_unit, int* a_unitId) {
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

	*a_unit = createUnit(classId, unitName);
	*a_unitId = unitId;
	for (int i = 0; i < nParams; i++) {
		double val;
		int prec;
		startPos = a_data->Get<double>(&val, startPos);
		startPos = a_data->Get<int>(&prec, startPos);
		if (*a_unit && (*a_unit)->hasParameter(i)) {
			(*a_unit)->setParameterValue(i, val);
			(*a_unit)->setParameterPrecision(i, prec);
		}
	}
	if (*a_unit) {
		for (int i = 0; i < nParams; i++) {
			(*a_unit)->onParamChange_(i);
		}
	}
	int reserved;
	startPos = a_data->Get<int>(&reserved, startPos);
	startPos += reserved;
	return startPos;
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