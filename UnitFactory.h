#pragma once
#include "Unit.h"
#include <vector>
#include <string>

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
    }

    void addSourceUnitPrototype(const SourceUnit* prototype)
    {
      m_source_unit_prototypes.push_back(prototype);
      m_source_prototype_names.push_back(prototype->getName());
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

    Unit* createUnit(int protonum) const
    {
      return m_unit_prototypes[protonum]->clone();
    }

    SourceUnit* createSourceUnit(int protonum) const
    {
      return dynamic_cast<SourceUnit*>(m_source_unit_prototypes[protonum]->clone());
    }

  protected:
    vector<const Unit*> m_unit_prototypes;
    vector<const SourceUnit*> m_source_unit_prototypes;
    vector<string> m_prototype_names;
    vector<string> m_source_prototype_names;
  };
}
