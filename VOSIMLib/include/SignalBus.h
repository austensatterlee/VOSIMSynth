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
        Signal(const Signal& a_other);
        Signal& operator=(const Signal& a_other);
        bool operator==(const Signal& a_rhs) const;

        double get() const;
        void set(const Signal& a_newVal);
        void set(const double& a_newVal);
        void clear();
    private:
        double m_value;
    };

    class SignalBus
    {
    public:
        int addChannel();
        int addChannel(const string& a_name);
        int getNumChannels() const;

        void set(SignalBus& a_other);

        template<typename ID>
        double getValue(const ID& a_identifier) const;

        template<typename ID>
        const Signal& getChannel(const ID& a_identifier) const;

        template<typename ID>
        Signal& getChannel(const ID& a_identifier);

        template <typename ID,typename T>
        bool setChannel(const ID& a_identifier, const T& a_newVal);

        template<typename ID>
        string getChannelName(const ID& a_identifier) const;

        void clear();
    private:
        NamedContainer<Signal> m_signals;
    };

    template<typename ID>
    const Signal& SignalBus::getChannel(const ID& a_identifier) const
    {
        return m_signals[a_identifier];
    }

    template<typename ID>
    Signal& SignalBus::getChannel(const ID& a_identifier)
    {
        return m_signals[a_identifier];
    }

    template<typename ID, typename T>
    bool SignalBus::setChannel(const ID& a_identifier, const T& a_newVal)
    {
        if(!m_signals.find(a_identifier))
            return false;
        Signal& sig = m_signals[a_identifier];
        sig.set(a_newVal);
        return true;
    }

    template<typename ID>
    double SignalBus::getValue(const ID& a_identifier) const
    {
        const Signal& signal = m_signals[a_identifier];
        return signal.get();
    }

    template<typename ID>
    string SignalBus::getChannelName(const ID& a_identifier) const{
        return m_signals.getItemName(a_identifier);
    }

}

#endif //VOSIMLIB_SIGNALBUS_H
