/// 
/// Created by austen on 2/5/2016.
///

#ifndef VOSIMLIB_SIGNALBUS_H
#define VOSIMLIB_SIGNALBUS_H

#include "NamedContainer.h"
#include <string>
#include <vector>
using std::vector;
using std::string;

namespace syn{
    class Signal {
    public:
        Signal();
        double get() const;
        void set(const Signal& a_newVal);
        void set(const double& a_newVal);
        void clear();
        bool operator==(const Signal& a_rhs) const{
            return m_value == a_rhs.m_value;
        }
    private:
        double m_value;
    };

    class SignalBus
    {
    public:
        int addChannel();
        int addChannel(const string& a_name);
        int size() const;

        void set(SignalBus& a_other);
        const NamedContainer<Signal>& get() const;

        template<typename ID>
        Signal& getChannel(const ID& a_identifier);

        template <typename ID,typename T>
        bool setChannel(const ID& a_identifier, const T& a_newVal);

        void clear();
    private:
        NamedContainer<Signal> m_signals;
    };

    template<typename ID>
    Signal& SignalBus::getChannel(const ID& a_identifier)
    {
        return m_signals.get(a_identifier);
    }

    template<typename ID, typename T>
    bool SignalBus::setChannel(const ID& a_identifier, const T& a_newVal)
    {
        if(!m_signals.find(a_identifier))
            return false;
        Signal& sig = m_signals.get(a_identifier);
        sig.set(a_newVal);
        return true;
    }

}

#endif //VOSIMLIB_SIGNALBUS_H
