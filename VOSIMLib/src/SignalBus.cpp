/*
Copyright 2016, Austen Satterlee

This file is part of VOSIMProject.

VOSIMProject is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

VOSIMProject is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with VOSIMProject. If not, see <http://www.gnu.org/licenses/>.
*/

#include "SignalBus.h"

namespace syn {
    int SignalBus::addChannel()
    {
        return m_signals.add("",Signal());
    }

    int SignalBus::addChannel(const string& a_name)
    {
        return m_signals.add(a_name, Signal());
    }

    int SignalBus::getNumChannels() const
    {
        return m_signals.size();
    }

    void SignalBus::set(SignalBus& a_other)
    {
		int nSignals = a_other.getNumChannels();
        for(int i=0;i<nSignals; i++){
            Signal& channel = a_other.getChannel(i);
            setChannel(i, channel);
        }
    }

    void SignalBus::clear()
    {
		int nSignals = m_signals.size();
        for(int i=0;i<nSignals;i++){
            m_signals[i].clear();
        }
    }

    /*---------------------*
     *  Signal methods
     *---------------------*/

    Signal::Signal() : m_value(0), m_default(0), m_accType(EAdd)
    { }

    bool Signal::operator==(const Signal& a_rhs) const
    {
        return m_value == a_rhs.m_value;
    }

	double Signal::get() const
    {
        return m_value;
    }

	void Signal::accumulate(const Signal& a_sig) {
	    switch(m_accType) {
	    case EAdd: 
			m_value += a_sig.m_value;
			break;
	    case EMul:
			m_value *= a_sig.m_value;
			break;
	    }
	}

	void Signal::set(const Signal& a_val)
    {
		*this = Signal(a_val);
    }

    void Signal::set(const double& a_newVal)
    {
        m_value = a_newVal;
    }

	void Signal::setChannelAccType(ChannelAccType a_accType) {
		m_accType = a_accType;
    }

	void Signal::setDefault(double a_default) {
		m_default = a_default;
    }

	void Signal::clear()
    {
        m_value = m_default;
    }
}
