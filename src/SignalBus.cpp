/// 
/// Created by austen on 2/5/2016.
///

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

    int SignalBus::size() const
    {
        return m_signals.size();
    }

    void SignalBus::set(SignalBus& a_other)
    {
        for(int i=0;i<a_other.size();i++){
            Signal& channel = a_other.getChannel(i);
            setChannel(i, channel);
        }
    }

    const NamedContainer<Signal>& SignalBus::get() const
    {
        return m_signals;
    }

    void SignalBus::clear()
    {
        for(int i=0;i<m_signals.size();i++){
            m_signals.get(i).clear();
        }
    }

    /*---------------------*
     *  Signal methods
     *---------------------*/

    Signal::Signal() : m_value(0)
    {
    }

    double Signal::get() const
    {
        return m_value;
    }

    void Signal::set(const Signal& a_val)
    {
        m_value = a_val.get();
    }

    void Signal::set(const double& a_newVal)
    {
        m_value = a_newVal;
    }

    void Signal::clear()
    {
        m_value = 0;
    }
}