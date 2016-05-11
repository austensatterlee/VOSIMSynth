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

#ifndef __Circuit__
#define __Circuit__

#include "Unit.h"
#include "NamedContainer.h"
#include <vector>
#include <memory>

using std::shared_ptr;
using std::vector;

namespace syn
{
	struct ConnectionRecord
	{
		int from_id;
		int from_port;
		int to_id;
		int to_port;

		bool operator==(const ConnectionRecord& a_other) const {
			bool res = (from_id == a_other.from_id && from_port == a_other.from_port && to_id == a_other.to_id && to_port == a_other.to_port);			
			return res;
		}
	};

	class PassthroughUnit : public Unit
	{
	public:
		explicit PassthroughUnit(const string& a_name) :
			Unit(a_name) {
		}
		PassthroughUnit(const PassthroughUnit& a_other) :
		PassthroughUnit(a_other.getName())
		{
		};
	protected:
		void MSFASTCALL process_(const SignalBus& a_inputs, SignalBus& a_outputs) GCCFASTCALL override {
			for(int i=0;i<a_inputs.getNumChannels();i++) {
				a_outputs.setChannel(i,a_inputs.getValue(i));
			}
		}
	private:
		inline string _getClassName() const override { return "_PassthroughUnit";  }
		Unit* _clone() const override { return new PassthroughUnit(*this); }
	};

	class OutputUnit : public Unit
	{
		
	};

	/**
	* \class Circuit
	*
	* \brief A collection of Units.
	*
	* A Circuit is a Unit that contains other Units.
	*
	*/
	class Circuit : public Unit
	{
	public:
		Circuit();

		explicit Circuit(const string& a_name);

		Circuit(const Circuit& a_other);

		/**
		 * \returns True if any of its internal units are active
		 */
		bool isActive() const override;

		const Unit& getUnit(int a_unitId) const;

		int getInputUnitId() const;
		int getOutputUnitId() const;

		template <typename UID>
		int getUnitId(const UID& a_unitIdentifier) const;

		int getNumUnits() const;

		void notifyMidiControlChange(int a_cc, double a_value);

		/**
		 * Retrieves a list of output ports connected to an input port.
		 * \returns A vector of pairs of the form {unit_id, port_id}
		 */
		template <typename UID>
		vector<pair<int, int>> getConnectionsToInternalInput(const UID& a_unitIdentifier, int a_portid) const;

		const vector<ConnectionRecord>& getConnections() const;

		template <typename UID, typename PID>
		double getInternalParameter(const UID& a_unitIdentifier, const PID& a_paramIdentifier) const;

		template <typename UID, typename PID, typename T>
		bool setInternalParameter(const UID& a_unitIdentifier, const PID& a_paramIdentifier, const T& a_value);

		template <typename UID, typename PID, typename T>
		bool setInternalParameterNorm(const UID& a_unitIdentifier, const PID& a_paramIdentifier, const T& a_value);

		template <typename UID, typename PID>
		bool setInternalParameterPrecision(const UID& a_unitIdentifier, const PID& a_paramIdentifier, int a_precision);

		/**
		 * \returns The id assigned to the newly added unit, or -1 on failure
		 */
		int addUnit(shared_ptr<Unit> a_unit);

		bool addUnit(shared_ptr<Unit> a_unit, int a_unitId);

		template <typename ID>
		bool removeUnit(const ID& a_identifier);

		template <typename ID>
		bool connectInternal(const ID& a_fromIdentifier, int a_fromOutputPort, const ID& a_toIdentifier, int
		                     a_toInputPort);

		template <typename ID>
		bool disconnectInternal(const ID& a_fromIdentifier, int a_fromOutputPort, const ID& a_toIdentifier, int
		                        a_toInputPort);

	protected:
		void MSFASTCALL process_(const SignalBus& a_inputs, SignalBus& a_outputs) GCCFASTCALL override;

		void onFsChange_() override;

		void onTempoChange_() override;

		void onNoteOn_() override;

		void onNoteOff_() override;

		void onMidiControlChange_(int a_cc, double a_value) override;

		Unit& getUnit_(int a_unitId);

		int addExternalInput_(const string& a_name);

		int addExternalOutput_(const string& a_name);

	private:
		string _getClassName() const override {
			return "Circuit";
		};

		Unit* _clone() const override;

	private:
		friend class VoiceManager;

		/**
		 *\todo make a collection class that automatically assigns an id to inserted elements. The id should be valid even when elements are deleted. Also allows user to request a specific id
		 **/
		NamedContainer<shared_ptr<Unit>> m_units;
		vector<ConnectionRecord> m_connectionRecords;
		shared_ptr<PassthroughUnit> m_inputUnit;
		shared_ptr<PassthroughUnit> m_outputUnit;
	};

	template <typename UID>
	vector<pair<int, int>> Circuit::getConnectionsToInternalInput(const UID& a_unitIdentifier, int a_portid) const {
		int unitId = m_units.getItemIndex(a_unitIdentifier);
		vector<pair<int, int>> connectedPorts;
		for (int i = 0; i < m_connectionRecords.size(); i++) {
			const ConnectionRecord& conn = m_connectionRecords[i];
			if (conn.to_id == unitId && conn.to_port == a_portid) {
				connectedPorts.push_back(make_pair(conn.from_id, conn.from_port));
			}
		}
		return connectedPorts;
	}

	template <typename UID>
	int Circuit::getUnitId(const UID& a_unitIdentifier) const {
		return m_units.getItemId(a_unitIdentifier);
	}

	template <typename UID, typename PID, typename T>
	bool Circuit::setInternalParameter(const UID& a_unitIdentifier, const PID& a_paramIdentifier, const T& a_value) {
		if (!m_units.find(a_unitIdentifier))
			return false;
		return m_units[a_unitIdentifier]->setParameter(a_paramIdentifier, a_value);
	};

	template <typename UID, typename PID, typename T>
	bool Circuit::setInternalParameterNorm(const UID& a_unitIdentifier, const PID& a_paramIdentifier, const T& a_value) {
		if (!m_units.find(a_unitIdentifier))
			return false;
		return m_units[a_unitIdentifier]->setParameterNorm(a_paramIdentifier, a_value);
	}

	template <typename UID, typename PID>
	bool Circuit::setInternalParameterPrecision(const UID& a_unitIdentifier, const PID& a_paramIdentifier, int a_precision) {
		if (!m_units.find(a_unitIdentifier))
			return false;
		return m_units[a_unitIdentifier]->setParameterPrecision(a_paramIdentifier, a_precision);
	};


	template <typename UID, typename PID>
	double Circuit::getInternalParameter(const UID& a_unitIdentifier, const PID& a_paramIdentifier) const {
		if (!m_units.find(a_unitIdentifier))
			return false;
		return m_units[a_unitIdentifier]->getParameter(a_paramIdentifier).getDouble();
	};

	template <typename ID>
	bool Circuit::removeUnit(const ID& a_unitIdentifier) {
		if (!m_units.find(a_unitIdentifier))
			return false;
		shared_ptr<Unit> unit = m_units[a_unitIdentifier];
		int unitId = m_units.getItemId(a_unitIdentifier);
		// Don't allow deletion of input or output unit
		if (unit == m_inputUnit || unit == m_outputUnit)
			return false;
		// Erase connections
		vector<ConnectionRecord> garbageList;
		for (int i = 0; i < m_connectionRecords.size(); i++) {
			const ConnectionRecord& rec = m_connectionRecords[i];
			if (unitId == rec.to_id || unitId == rec.from_id) {
				garbageList.push_back(rec);
			}
		}
		for(int i=0;i<garbageList.size();i++) {
			const ConnectionRecord& rec = garbageList[i];
			disconnectInternal(rec.from_id, rec.from_port, rec.to_id, rec.to_port);
		}
		m_units.remove(a_unitIdentifier);
		return true;
	}

	template <typename ID>
	bool Circuit::connectInternal(const ID& a_fromIdentifier, int a_fromOutputPort, const ID& a_toIdentifier,
	                              int a_toInputPort) {
		int fromUnitId = m_units.getItemId(a_fromIdentifier);
		int toUnitId = m_units.getItemId(a_toIdentifier);
		if (!m_units.find(fromUnitId) || !m_units.find(toUnitId))
			return false;

		shared_ptr<Unit> fromUnit = m_units[fromUnitId];
		shared_ptr<Unit> toUnit = m_units[toUnitId];

		if (fromUnit->getNumOutputs() <= a_fromOutputPort || toUnit->getNumInputs() <= a_toInputPort)
			return false;

		bool result = toUnit->_connectInput(fromUnit, a_fromOutputPort, a_toInputPort);
		// record the connection upon success
		if (result) {
			m_connectionRecords.push_back({fromUnitId, a_fromOutputPort, toUnitId,a_toInputPort});
		}
		return result;
	}

	template <typename ID>
	bool Circuit::disconnectInternal(const ID& a_fromIdentifier, int a_fromOutputPort, const ID& a_toIdentifier,
	                                 int a_toInputPort) {
		int fromId = m_units.getItemId(a_fromIdentifier);
		int toId = m_units.getItemId(a_toIdentifier);
		if (!m_units.find(fromId) || !m_units.find(toId))
			return false;

		shared_ptr<Unit> fromUnit = m_units[fromId];
		shared_ptr<Unit> toUnit = m_units[toId];

		if (fromUnit->getNumOutputs() <= a_fromOutputPort || toUnit->getNumInputs() <= a_toInputPort)
			return false;

		bool result = toUnit->_disconnectInput(fromUnit, a_fromOutputPort, a_toInputPort);
		// find and remove the associated connection record upon successful removal
		if (result) {
			ConnectionRecord record = {fromId, a_fromOutputPort, toId, a_toInputPort};
			for (unsigned i = 0; i < m_connectionRecords.size(); i++) {
				if (m_connectionRecords[i] == record) {
					m_connectionRecords.erase(m_connectionRecords.begin() + i);
					break;
				}
			}
		}
		return result;
	}
};
#endif // __Circuit__


