/// 
/// Created by austen on 2/5/2016.
///

#ifndef VOSIMLIB_CONNECTIONBUS_H
#define VOSIMLIB_CONNECTIONBUS_H

#include "SignalBus.h"
#include <cstring>
#include <memory>

using std::shared_ptr;
using std::weak_ptr;

namespace syn
{
    class Unit;

    struct UnitConnector {
        shared_ptr<Unit> connectedUnit;
        int connectedPort;
        int localPort;
        bool operator==(const UnitConnector& a_rhs) const {
            return memcmp(this, &a_rhs, sizeof(UnitConnector))==0;
        }
    };

    class UnitConnectionBus
    {
    public:
        /**
         * Create the specified connection
         * \returns False if connection already existed
         */
        bool connect(shared_ptr<Unit> a_connectedUnit, int a_connectedPort, int a_localPort);
        /**
         * Remove the specified connection
         * \returns True if connection existed
         */
        bool disconnect(shared_ptr<Unit> a_connectedUnit, int a_connectedPort, int a_localPort);
        /**
         * Remove all connections that reference the specified unit
         * \returns True if any connections were removed
         */
        bool disconnect(shared_ptr<Unit> a_connectedUnit);
        /**
         * Pull incoming output signals from the specified port into the specified receiving signal.
         * If multiple connections are present at a port, their values are summed.
         */
        bool pull(int a_localPort, Signal& a_recipient) const;
        /**
        * Push the specified output signal into the input signals connected to the specified port.
        */
        bool push(int a_localPort, Signal& a_signal) const;

        int numPorts() const;

    private:
        typedef vector<UnitConnector> UnitPort;
        vector<UnitPort> m_ports;
    };
}

#endif //VOSIMLIB_CONNECTIONBUS_H
