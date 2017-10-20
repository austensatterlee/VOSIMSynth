#include "vosimlib/UnitFactory.h"
#include "vosimlib/Unit.h"
#include "vosimlib/DSPMath.h"

syn::UnitFactory::FactoryPrototype::FactoryPrototype(string a_group_name, Unit* a_unit, size_t a_class_size)
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

string syn::UnitFactory::getPrototypeName(UnitTypeId a_classIdentifier) const {
    if(!hasPrototype(a_classIdentifier))
        return "";
    return getPrototype(a_classIdentifier).name;
}

syn::Unit* syn::UnitFactory::createUnit(UnitTypeId a_classIdentifier, const string& a_name) {
    if (!hasPrototype(a_classIdentifier))
        return nullptr;
    FactoryPrototype& p = getPrototype(a_classIdentifier);
    Unit* unit = p.prototype->clone();
    p.build_count++;
    // Generate default name
    if (a_name.empty())
        unit->setName(generateUnitName(unit->getClassIdentifier()));
    else
        unit->setName(a_name);
    return unit;
}

string syn::UnitFactory::generateUnitName(UnitTypeId a_classIdentifier) const
{
    if (!hasPrototype(a_classIdentifier))
        return "";
    const FactoryPrototype& p = getPrototype(a_classIdentifier);
    return p.name;
}

bool syn::UnitFactory::hasPrototype(UnitTypeId a_classIdentifier) const {
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

syn::UnitFactory::FactoryPrototype& syn::UnitFactory::getPrototype(UnitTypeId a_classIdentifier) {
    return const_cast<FactoryPrototype&>(static_cast<const UnitFactory*>(this)->getPrototype(a_classIdentifier));
}

const syn::UnitFactory::FactoryPrototype& syn::UnitFactory::getPrototype(UnitTypeId a_classIdentifier) const {
    for (const FactoryPrototype& prototype : m_prototypes) {
        if (prototype.classIdentifier == a_classIdentifier)
            return prototype;
    }
    throw std::runtime_error("Prototype not found.");
}
