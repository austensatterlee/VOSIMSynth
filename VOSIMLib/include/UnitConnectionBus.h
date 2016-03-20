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


#ifndef __UNITCONNECTIONBUS__
#define __UNITCONNECTIONBUS__

#include "SignalBus.h"
#include <memory>

using std::shared_ptr;

namespace syn
{
    class Unit;

    struct UnitConnector {
        shared_ptr<Unit> connectedUnit;
        int connectedPort;
        int localPort;
        bool operator==(const UnitConnector& a_rhs) const {
			return connectedUnit == a_rhs.connectedUnit && connectedPort == a_rhs.connectedPort && localPort == a_rhs.localPort;
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
        void pull(int a_localPort, Signal& a_recipient) const;
        /**
        * Push the specified output signal into the input signals connected to the specified port.
        */
        void push(int a_localPort, Signal& a_signal) const;

		void addPort() { m_ports.push_back({}); m_numPorts += 1;  };

        int numPorts() const;
	    int numConnections(int a_portNum);
    private:
        typedef vector<UnitConnector> UnitPort;
        vector<UnitPort> m_ports;
		int m_numPorts = 0;
    };
}

#endif //VOSIMLIB_CONNECTIONBUS_H
