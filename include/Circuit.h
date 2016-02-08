#ifndef __Circuit__
#define __Circuit__

#include "Unit.h"
#include "NamedContainer.h"
#include <vector>
#include <memory>

using std::shared_ptr;
using std::vector;

namespace syn {
    /**
    * \class Circuit
    *
    * \brief A collection of Units.
    *
    * A Circuit is a Unit that contains other Units.
    *
    */
    class Circuit : public Cloneable<Unit,Circuit> {
    public:
        Circuit();

        int addUnit(shared_ptr<Unit> a_unit);

        template<typename ID>
        bool removeUnit(const ID& a_identifier);

        template<typename ID>
        bool connectInputs(int a_circuitInputPort, const ID& a_toIdentifier, int a_toInputPort);

        template<typename ID>
        bool connectOutputs(int a_circuitOutputPort, const ID& a_fromIdentifier, int a_fromOutputPort);

        template<typename ID>
        bool connectInternal(const ID& a_fromIdentifier, int a_fromOutputPort, const ID& a_toIdentifier, int
        a_toInputPort);

        template<typename ID>
        bool disconnectInputs(int a_circuitInputPort, const ID& a_toIdentifier, int a_toInputPort);

        template<typename ID>
        bool disconnectOutputs(int a_circuitOutputPort, const ID& a_fromIdentifier, int a_fromOutputPort);

        template<typename ID>
        bool disconnectInternal(const ID& a_fromIdentifier, int a_fromOutputPort, const ID& a_toIdentifier, int
        a_toInputPort);
    protected:
        virtual bool onParamChange_(int a_paramId) override;
        virtual void process_(const SignalBus& a_inputs, SignalBus& a_outputs) override;
    private:
        virtual string _getClassName() const override { return "Circuit"; };
    private:
        NamedContainer<shared_ptr<Unit> > m_units;
        UnitConnectionBus m_internalInputs;
        UnitConnectionBus m_internalOutputs;
    };

    template<typename ID>
    bool Circuit::removeUnit(const ID& a_identifier)
    {
        return m_units.remove(a_identifier);
    }

    template<typename ID>
    bool Circuit::connectInputs(int a_circuitInputPort, const ID& a_toIdentifier, int a_toInputPort)
    {
        if( !m_units.find(a_toIdentifier) )
            return false;

        shared_ptr<Unit> toUnit = *(m_units.get(a_toIdentifier));

        if( toUnit.get()->getNumInputs() <= a_toInputPort || getNumInputs() <= a_circuitInputPort)
            return false;

        return m_internalInputs.connect(toUnit, a_toInputPort, a_circuitInputPort);
    }

    template<typename ID>
    bool Circuit::connectOutputs(int a_circuitOutputPort, const ID& a_fromIdentifier, int a_fromOutputPort)
    {
        if( !m_units.find(a_fromIdentifier) )
            return false;

        shared_ptr<Unit> fromUnit = m_units.get(a_fromIdentifier);

        if( fromUnit.get()->getNumOutputs() <= a_fromOutputPort || getNumOutputs() <= a_circuitOutputPort)
            return false;

        return m_internalOutputs.connect(fromUnit, a_fromOutputPort, a_circuitOutputPort);
    }

    template<typename ID>
    bool Circuit::connectInternal(const ID& a_fromIdentifier, int a_fromOutputPort, const ID& a_toIdentifier,
                                  int a_toInputPort)
    {
        if( !m_units.find(a_fromIdentifier) || !m_units.find(a_toIdentifier) )
            return false;

        shared_ptr<Unit> fromUnit = *(m_units.get(a_fromIdentifier));
        shared_ptr<Unit> toUnit = *(m_units.get(a_toIdentifier));

        if( fromUnit.get()->getNumOutputs() <= a_fromOutputPort || toUnit.get()->getNumInputs() <= a_toInputPort)
            return false;

        return toUnit.get()->_connectInput(fromUnit, a_fromOutputPort, a_toInputPort);
    }

    template<typename ID>
    bool Circuit::disconnectInputs(int a_circuitInputPort, const ID& a_toIdentifier, int a_toInputPort)
    {
        if( !m_units.find(a_toIdentifier) )
            return false;

        shared_ptr<Unit> toUnit = *(m_units.get(a_toIdentifier));

        if( toUnit.get()->getNumInputs() <= a_toInputPort || getNumInputs() <= a_circuitInputPort)
            return false;

        return m_internalInputs.disconnect(toUnit, a_toInputPort, a_circuitInputPort);
    }

    template<typename ID>
    bool Circuit::disconnectOutputs(int a_circuitOutputPort, const ID& a_fromIdentifier, int a_fromOutputPort)
    {
        if( !m_units.find(a_fromIdentifier) )
            return false;

        shared_ptr<Unit> fromUnit = *(m_units.get(a_fromIdentifier));

        if( fromUnit.get()->getNumOutputs() <= a_fromOutputPort || getNumOutputs() <= a_circuitOutputPort)
            return false;

        return m_internalOutputs.disconnect(fromUnit, a_fromOutputPort, a_circuitOutputPort);
    }

    template<typename ID>
    bool Circuit::disconnectInternal(const ID& a_fromIdentifier, int a_fromOutputPort, const ID& a_toIdentifier,
                                     int a_toInputPort)
    {
        if( !m_units.find(a_fromIdentifier) || !m_units.find(a_toIdentifier) )
            return false;

        shared_ptr<Unit> fromUnit = *(m_units.get(a_fromIdentifier));
        shared_ptr<Unit> toUnit = *(m_units.get(a_toIdentifier));

        if( fromUnit.get()->getNumOutputs() <= a_fromOutputPort || toUnit.get()->getNumInputs() <= a_toInputPort)
            return false;

        return toUnit.get()->_disconnectInput(fromUnit, a_fromOutputPort, a_toInputPort);
    }
};
#endif // __Circuit__