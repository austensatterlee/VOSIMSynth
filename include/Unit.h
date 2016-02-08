#ifndef __UNIT__
#define __UNIT__

#include "UnitParameter.h"
#include "SignalBus.h"
#include "UnitConnectionBus.h"
#include "NamedContainer.h"
#include <vector>
#include <memory>

using std::shared_ptr;
using std::make_shared;

namespace syn {
    class Circuit;
    /**
     * \class Unit
     *
     * \brief Units encapsulate a discrete processor with an internal state
     *
     * A unit is composed of internal state variables, a number of outputs and inputs,
     * and a transition function, Unit::process_, which produces a new set of outputs given the state
     * of the Unit and the current input.
     *
     * New units should be derived by subclassing UnitCloneable, with Unit::addInput_, Unit::addOutput_,
     * and Unit::addParameter_ called in the constructor some of number of times according to
     * how many inputs, outputs, and parameters are needed (respectively).
     *
     */
    class Unit {
    public:
        Unit() :
                m_parent{nullptr},
                m_name{""},
                m_hasTicked(false)
        {}

        Unit(const string& a_name) :
                m_parent{nullptr},
                m_name{a_name},
                m_hasTicked(false)
        {}

        Unit(const Unit& a_other);

        virtual ~Unit(){};

        void tick();

        void reset();

        shared_ptr<Circuit> getParent();

        template<typename ID>
        const UnitParameter& getParameter(const ID& a_identifier);

        template<typename ID, typename T>
        bool setParameter(const ID& a_identifier, const T& a_value);

        SignalBus& getOutputBus();
        SignalBus& getInputBus();

        int getNumParameters() const;
        int getNumInputs() const;
        int getNumOutputs() const;

        const string& getName() const;

        virtual Unit* clone() const = 0;

        unsigned int getClassIdentifier() const;

    protected:
        /**
         * Called when a parameter has been modified. This function should be overridden
         * to update the internal state and verify that the change is valid. If the change is not valid,
         * the function should return false.
         */
        virtual bool onParamChange_(int a_paramId) = 0;

        virtual void process_(const SignalBus& a_inputs, SignalBus& a_outputs) = 0;

        int addInput_(const string& a_name);

        int addOutput_(const string& a_name);

        int addParameter_(const UnitParameter& a_param);

    private:
        virtual inline string _getClassName() const = 0;

        void _setParent(shared_ptr<Circuit> a_new_parent);

        void _setName(const string& a_name);

        bool _connectInput(shared_ptr<Unit> a_fromUnit, int a_fromOutputPort, int a_toInputPort);

        bool _disconnectInput(shared_ptr<Unit> a_fromUnit, int a_fromOutputPort, int a_toInputPort);

    private:
        friend class Circuit;
        friend class UnitFactory;

        string m_name;
        bool m_hasTicked;
        NamedContainer<UnitParameter> m_parameters;
        SignalBus m_inputSignals;
        SignalBus m_outputSignals;
        UnitConnectionBus m_inputConnections;
        shared_ptr<Circuit> m_parent;
    };

    template<typename ID>
    const UnitParameter& Unit::getParameter(const ID& a_identifier) {
        return m_parameters.get(a_identifier);
    }

    template<typename ID, typename T>
    bool Unit::setParameter(const ID& a_identifier, const T& a_value){
        if(!m_parameters.find(a_identifier))
            return false;
        if(m_parameters.get(a_identifier).set(a_value)){
            int id = m_parameters.getItemIndex(a_identifier);
            onParamChange_(id);
            return true;
        }else{
            return false;
        }
    };

    template<typename Base,typename Derived>
    class Cloneable : public Base {
    public:
        template<typename... Args>
        explicit Cloneable(Args... args) : Base(args...)
        {
        }

        virtual ~Cloneable() {};

        virtual Base* clone() const
        {
            return new Derived(dynamic_cast<Derived const &>(*this));
        }
    };
}
#endif

