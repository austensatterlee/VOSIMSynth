#include "Unit.h"

using std::hash;

using namespace std;

namespace syn
{

    Unit::Unit(const Unit& a_other) :
    m_parent(a_other.m_parent),
    m_name(a_other.m_name),
    m_inputSignals(a_other.m_inputSignals),
    m_outputSignals(a_other.m_outputSignals)
    {

    }

    unsigned int Unit::getClassIdentifier() const {
        hash<string> hash_fn;
        return hash_fn(_getClassName());
    }

    shared_ptr<Circuit> Unit::getParent() {
        return m_parent;
    }

    SignalBus& Unit::getOutputBus()
    {
        return m_outputSignals;
    }

    SignalBus& Unit::getInputBus()
    {
        return m_inputSignals;
    }

    int Unit::getNumParameters() const
    {
        return m_parameters.size();
    }

    int Unit::getNumInputs() const
    {
        return m_inputSignals.size();
    }

    int Unit::getNumOutputs() const
    {
        return m_outputSignals.size();
    }

    const string& Unit::getName() const {
        return m_name;
    }

    void Unit::_setName(const string& a_name) {
        m_name = a_name;
    }

    void Unit::tick() {
        if(m_hasTicked)
            return;
        m_hasTicked = true;

        /* Pull signals from inputs */
        for( int i = 0; i < m_inputSignals.size(); i++){
            Signal& inputChannel = m_inputSignals.getChannel(i);
            m_inputConnections.pull(i,inputChannel);
        }

        m_outputSignals.clear();

        process_(m_inputSignals, m_outputSignals);
    }

    void Unit::reset() {
        m_inputSignals.clear();
        m_hasTicked = false;
    }

    int Unit::addInput_(const string& a_name) {
        return m_inputSignals.addChannel(a_name);
    }

    int Unit::addOutput_(const string& a_name) {
        return m_outputSignals.addChannel(a_name);
    }

    int Unit::addParameter_(const UnitParameter& a_param) {
        int indx = m_parameters.add(a_param.getName(), a_param);
        return m_parameters.getItemIndex(indx);
    }

    void Unit::_setParent(shared_ptr<Circuit> a_new_parent) {
        m_parent = a_new_parent;
    }

    bool Unit::_connectInput(shared_ptr<Unit> a_fromUnit, int a_fromOutputPort, int a_toInputPort) {
        return m_inputConnections.connect(a_fromUnit, a_fromOutputPort, a_toInputPort);
    }

    bool Unit::_disconnectInput(shared_ptr<Unit> a_fromUnit, int a_fromOutputPort, int a_toInputPort) {
        return m_inputConnections.disconnect(a_fromUnit, a_fromOutputPort, a_toInputPort);
    }

}

