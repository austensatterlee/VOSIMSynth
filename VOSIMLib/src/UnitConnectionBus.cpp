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

#include "UnitConnectionBus.h"
#include "Unit.h"

namespace syn
{
	bool UnitConnectionBus::connect(shared_ptr<Unit> a_connectedUnit, int a_connectedPort, int a_localPort) {
		UnitConnector conn{ a_connectedUnit, a_connectedPort, a_localPort };

		/* Make sure we don't add duplicate connections */
		UnitPort& port = m_ports[a_localPort];
		for (int i = 0; i < port.size(); i++) {
			if (port[i] == conn) {
				return false;
			}
		}
		port.push_back(conn);
		return true;
	}

	bool UnitConnectionBus::disconnect(shared_ptr<Unit> a_connectedUnit, int a_connectedPort, int a_localPort) {
		UnitConnector conn{ a_connectedUnit, a_connectedPort, a_localPort };

		if (m_ports.size() <= a_localPort) {
			return false;
		}

		/* Search for connection in input port */
		UnitPort& port = m_ports[a_localPort];
		for (int i = 0; i < port.size(); i++) {
			if (port[i] == conn) {
				port.erase(port.begin() + i);
				return true;
			}
		}
		return false;
	}

	void UnitConnectionBus::pull(int a_localPort, Signal& a_recipient) const {
		const UnitPort& port = m_ports[a_localPort];
		int pSize = port.size();
		a_recipient.clear();
		for (int i = 0; i < pSize; i++) {
			const UnitConnector& conn = port[i];

			// add outputChannel to a_recipient
			const Signal& outputChannel = conn.connectedUnit->getOutputChannel(conn.connectedPort);
			a_recipient.accumulate(outputChannel);
		}
	}

	void UnitConnectionBus::push(int a_localPort, Signal& a_signal) const {
		const UnitPort& port = m_ports[a_localPort];
		int pSize = port.size();
		for (int i = 0; i < pSize; i++) {
			// add outputChannel to a_recipient
			port[i].connectedUnit->m_inputSignals.getChannel(port[i].connectedPort).accumulate(a_signal);
		}
	}

	const UnitConnectionBus::UnitPort& UnitConnectionBus::getPort(int a_index) const
	{
		return m_ports[a_index];
	}

	int UnitConnectionBus::numPorts() const {
		return m_numPorts;
	}

	int UnitConnectionBus::numConnections(int a_portNum) const {
		if (m_numPorts > a_portNum)
			return m_ports[a_portNum].size();
		return 0;
	}

	bool UnitConnectionBus::disconnect(shared_ptr<Unit> a_connectedUnit) {
		vector<pair<int, int>> garbage_list;
		// Find all connections referencing this unit and add them to the garbage list
		for (int i = 0; i < m_numPorts; i++) {
			for (int j = 0; j < m_numPorts; j++) {
				if (m_ports[i][j].connectedUnit == a_connectedUnit) {
					garbage_list.push_back(make_pair(i, j));
				}
			}
		}
		// Remove all connections in the garbage list
		for (int i = 0; i < garbage_list.size(); i++) {
			UnitPort& port = m_ports[garbage_list[i].first];
			port.erase(port.begin() + garbage_list[i].second);
		}

		return garbage_list.size() > 0;
	}
}