#include "UnitFactory.h"
#include "Unit.h"
#include "DSPMath.h"

syn::UnitFactory::FactoryPrototype::FactoryPrototype(std::string a_group_name, syn::Unit* a_unit, size_t a_class_size)
    : classIdentifier(a_unit->getClassIdentifier()),
      group_name(a_group_name),
      name(a_unit->name()),
      prototype(a_unit),
      build_count{0},
      size(a_class_size) {}

syn::UnitFactory::~UnitFactory()
{
    for (auto proto : m_prototypes)
    {
        delete proto.prototype;
    }
}

const vector<string>& syn::UnitFactory::getGroupNames() const {
    return m_group_names;
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

const syn::UnitFactory::FactoryPrototype* syn::UnitFactory::getFactoryPrototype(UnitTypeId a_classIdentifier) const {
    for (const FactoryPrototype& prototype : m_prototypes) {
        if (prototype.classIdentifier == a_classIdentifier)
            return& prototype;
    }
    return nullptr;
}

syn::Unit* syn::UnitFactory::createUnit_(int a_protoNum, const string& a_name) {
    Unit* unit = m_prototypes[a_protoNum].prototype->clone();
    // Generate default name
    if(a_name.empty())
        unit->_setName(generateUnitName(unit->getClassIdentifier()));
    else
        unit->_setName(a_name);
    return unit;
}

syn::Unit* syn::UnitFactory::createUnit(syn::UnitTypeId a_classIdentifier, const string& a_name) {
    int protonum = getPrototypeIdx_(a_classIdentifier);
    if (protonum < 0)
        return nullptr;
    return createUnit_(protonum, a_name);
}

std::string syn::UnitFactory::generateUnitName(syn::UnitTypeId a_classIdentifier) const
{
    const FactoryPrototype* p = getFactoryPrototype(a_classIdentifier);
    return p->name+std::to_string(p->build_count);
}

bool syn::UnitFactory::hasClassId(syn::UnitTypeId a_classIdentifier) const {
    bool result = m_class_identifiers.find(a_classIdentifier) != m_class_identifiers.end();
    return result;
}

syn::UnitTypeId syn::UnitFactory::getClassId(const string& a_groupName, const string& a_protoName) const {
    for (const FactoryPrototype& prototype : m_prototypes) {
        if(prototype.group_name==a_groupName && prototype.name==a_protoName) {
            return prototype.classIdentifier;
        }
    }
    return 0;
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

int syn::UnitFactory::getPrototypeIdx_(syn::UnitTypeId a_classId) const {
    if (m_class_identifiers.find(a_classId) != m_class_identifiers.end()) {
        return m_class_identifiers.at(a_classId);
    }
    return -1;
}
