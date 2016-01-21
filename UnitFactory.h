#pragma once
#include "Unit.h"
#include "SourceUnit.h"
#include <vector>

using std::vector;

namespace syn
{
  class UnitFactory
  {
  public:

    UnitFactory()
    {}

    virtual ~UnitFactory()
    {
      for (int i = 0; i < m_unit_prototypes.size(); i++)
      {
        delete m_unit_prototypes[i];
      }
    }

    /**
     * \brief Register a prototype unit with the factory. Prototype deletion will be taken care of upon factory destruction.
     */
    void addUnitPrototype(const Unit* prototype)
    {
      m_unit_prototypes.push_back(prototype);
      m_prototype_names.push_back(prototype->getName());
      m_unit_counts.push_back(0);
      m_class_identifiers[prototype->getClassIdentifier()] = m_unit_prototypes.size() - 1;
    }

    void addSourceUnitPrototype(const SourceUnit* prototype)
    {
      m_source_unit_prototypes.push_back(prototype);
      m_source_prototype_names.push_back(prototype->getName());
      m_source_unit_counts.push_back(0);
      m_class_identifiers[prototype->getClassIdentifier()] = m_source_unit_prototypes.size() - 1;
    }

    const vector<string>& getPrototypeNames() const
    {
      return m_prototype_names;
    }

    const vector<string>& getSourcePrototypeNames() const
    {
      return m_source_prototype_names;
    }

    const vector<const Unit*>& getPrototypes() const
    {
      return m_unit_prototypes;
    }

    const vector<const SourceUnit*>& getSourcePrototypes() const
    {
      return m_source_unit_prototypes;
    }

    Unit* createUnit(int protonum)
    {
      Unit* unit = m_unit_prototypes[protonum]->clone();
      char namebuf[256];
      sprintf(namebuf, "%s_%d", unit->getName().c_str(), m_unit_counts[protonum]);
      string newname = namebuf;
      unit->setName(newname);
      m_unit_counts[protonum]++;
      return unit;
    }

    SourceUnit* createSourceUnit(int protonum)
    {
      SourceUnit* srcunit = dynamic_cast<SourceUnit*>(m_source_unit_prototypes[protonum]->clone());
      char namebuf[256];
      sprintf(namebuf, "%s_%d", srcunit->getName().c_str(), m_source_unit_counts[protonum]);
      string newname = namebuf;
      srcunit->setName(newname);
      m_source_unit_counts[protonum]++;
      return srcunit;
    }

    SourceUnit* createSourceUnit(const unsigned int classidentifier) {
      int protonum = m_class_identifiers.at(classidentifier);
      return createSourceUnit(protonum);
    }

    Unit* createUnit(const unsigned int classidentifier) {
      int protonum = m_class_identifiers.at(classidentifier);
      return createUnit(protonum);
    }

	bool hasClassId(unsigned a_unit_class_id) {
		bool result = m_class_identifiers.find(a_unit_class_id) != m_class_identifiers.end();
		return result;
    }
  protected:
    vector<const Unit*> m_unit_prototypes;
    vector<int> m_unit_counts;
    vector<const SourceUnit*> m_source_unit_prototypes;
    vector<int> m_source_unit_counts;
    vector<string> m_prototype_names;
    vector<string> m_source_prototype_names;
    unordered_map<unsigned int, int> m_class_identifiers;
  };
}
