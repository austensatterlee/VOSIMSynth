#include "Circuit.h"
#include <stdexcept>
#include <vector>
#include <unordered_set>

using std::vector;
using std::unordered_set;

namespace syn
{
	void Circuit::process_(const SignalBus& inputs, SignalBus& outputs)
	{
        /* Push circuit input signals to internally connected input ports */
        for(int i=0;i<m_inputSignals.size();i++){
            Signal& inputSignal = m_inputSignals.getChannel(i);
            m_internalInputs.push(i, inputSignal);
        }

        // reset all internal components
        for (int i = 0; i < m_units.size(); i++)
        {
            m_units.get(i).get()->reset();
        }

        /* Push internally connected output signals to circuit output ports */
        for(int i=0;i<m_outputSignals.size();i++){
            Signal& outputSignal = m_outputSignals.getChannel(i);
            m_internalOutputs.pull(i, outputSignal);
        }
	}

    Circuit::Circuit()
    {
        addInput_("in");
        addOutput_("out");
    }

    int Circuit::addUnit(shared_ptr<Unit> a_unit)
    {
        return m_units.add(a_unit.get()->getName(),a_unit);
    }

    bool Circuit::onParamChange_(int a_paramId)
    {
        return false;
    }
}

