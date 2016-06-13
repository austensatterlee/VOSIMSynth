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
#include <list>

using std::vector;
using std::list;

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
			Unit(a_name) { }

		PassthroughUnit(const PassthroughUnit& a_other) :
			PassthroughUnit(a_other.getName()) { };

	protected:
		void MSFASTCALL process_() GCCFASTCALL override {
			for (int i = 0; i < getNumInputs(); i++) {
				setOutputChannel_(i, getInputValue(i));
			}
		}

	private:
		inline string _getClassName() const override {
			return "_PassthroughUnit";
		}

		Unit* _clone() const override {
			return new PassthroughUnit(*this);
		}
	};

	class OutputUnit : public Unit { };

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

		virtual ~Circuit();

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

		const list<Unit*>& getProcGraph() const;

		void notifyMidiControlChange(int a_cc, double a_value);

		/**
		 * Retrieves a list of output ports connected to an input port.
		 * \returns A vector of (unit_id, port_id) pairs.
		 */
		vector<std::pair<int, int>> getConnectionsToInternalInput(int a_unitId, int a_portid) const;

		/**
		 * Get a list of all connections within the Circuit.
		 */
		const vector<ConnectionRecord>& getConnections() const;

		template <typename UID, typename PID>
		double getInternalParameter(const UID& a_unitIdentifier, const PID& a_paramIdentifier) const;

		template <typename UID, typename PID, typename T>
		bool setInternalParameter(const UID& a_unitIdentifier, const PID& a_paramIdentifier, const T& a_value);

		template <typename UID, typename PID, typename T>
		bool setInternalParameterNorm(const UID& a_unitIdentifier, const PID& a_paramIdentifier, const T& a_value);

		template <typename UID, typename PID>
		bool setInternalParameterPrecision(const UID& a_unitIdentifier, const PID& a_paramIdentifier, int a_precision);

		template <typename UID, typename PID>
		bool setInternalParameterFromString(const UID& a_unitIdentifier, const PID& a_paramIdentifier, const string& a_value);

		/**
		 * \returns The id assigned to the newly added unit, or -1 on failure
		 */
		int addUnit(Unit* a_unit);

		/**
		 * \returns True if the unit was added to the circuit (i.e. the provided id was not already taken).
		 */
		bool addUnit(Unit* a_unit, int a_unitId);

		template <typename ID>
		bool removeUnit(const ID& a_identifier);

		template <typename ID>
		bool connectInternal(const ID& a_fromIdentifier, int a_fromOutputPort, const ID& a_toIdentifier, int
			a_toInputPort);

		template <typename ID>
		bool disconnectInternal(const ID& a_fromIdentifier, int a_fromOutputPort, const ID& a_toIdentifier, int
			a_toInputPort);

	protected:
		void MSFASTCALL process_() GCCFASTCALL override;

		void onFsChange_() override;

		void onTempoChange_() override;

		void onNoteOn_() override;

		void onNoteOff_() override;

		void onMidiControlChange_(int a_cc, double a_value) override;

		void onInputConnection_(int a_inputPort) override;

		void onInputDisconnection_(int a_inputPort) override;

		Unit& getUnit_(int a_unitId);

		int addExternalInput_(const string& a_name);

		int addExternalOutput_(const string& a_name);

	private:
		string _getClassName() const override {
			return "Circuit";
		};

		Unit* _clone() const override;

		void _recomputeGraph();

	private:
		friend class VoiceManager;

		NamedContainer<Unit*, 64> m_units;
		vector<ConnectionRecord> m_connectionRecords;
		PassthroughUnit* m_inputUnit;
		PassthroughUnit* m_outputUnit;

		list<Unit*> m_procGraph;
	};

	template <typename UID>
	int Circuit::getUnitId(const UID& a_unitIdentifier) const {
		return m_units.find(a_unitIdentifier);
	}

	template <typename UID, typename PID, typename T>
	bool Circuit::setInternalParameter(const UID& a_unitIdentifier, const PID& a_paramIdentifier, const T& a_value) {
		return m_units[a_unitIdentifier]->setParameter(a_paramIdentifier, a_value);
	};

	template <typename UID, typename PID, typename T>
	bool Circuit::setInternalParameterNorm(const UID& a_unitIdentifier, const PID& a_paramIdentifier, const T& a_value) {
		return m_units[a_unitIdentifier]->setParameterNorm(a_paramIdentifier, a_value);
	}

	template <typename UID, typename PID>
	bool Circuit::setInternalParameterPrecision(const UID& a_unitIdentifier, const PID& a_paramIdentifier, int a_precision) {
		return m_units[a_unitIdentifier]->setParameterPrecision(a_paramIdentifier, a_precision);
	}

	template <typename UID, typename PID>
	bool Circuit::setInternalParameterFromString(const UID& a_unitIdentifier, const PID& a_paramIdentifier, const string& a_value) {
		return m_units[a_unitIdentifier]->setParameterFromString(a_paramIdentifier, a_value);
	};

	template <typename UID, typename PID>
	double Circuit::getInternalParameter(const UID& a_unitIdentifier, const PID& a_paramIdentifier) const {
		return m_units[a_unitIdentifier]->getParameter(a_paramIdentifier).getDouble();
	};

	template <typename ID>
	bool Circuit::removeUnit(const ID& a_unitIdentifier) {
		if (!m_units.find(a_unitIdentifier))
			return false;
		int unitId = m_units.find(a_unitIdentifier);
		Unit* unit = m_units[unitId];
		// Don't allow deletion of input or output unit
		if (unit == m_inputUnit || unit == m_outputUnit)
			return false;
		// Erase connections
		vector<ConnectionRecord> garbageList;
		const int nRecords = m_connectionRecords.size();
		for (int i = 0; i < nRecords; i++) {
			const ConnectionRecord& rec = m_connectionRecords[i];
			if (unitId == rec.to_id || unitId == rec.from_id) {
				garbageList.push_back(rec);
			}
		}
		for (int i = 0; i < garbageList.size(); i++) {
			const ConnectionRecord& rec = garbageList[i];
			disconnectInternal(rec.from_id, rec.from_port, rec.to_id, rec.to_port);
		}
		m_units.remove(a_unitIdentifier);
		_recomputeGraph();
		return true;
	}

	template <typename ID>
	bool Circuit::connectInternal(const ID& a_fromIdentifier, int a_fromOutputPort, const ID& a_toIdentifier,
		int a_toInputPort) {
		int fromUnitId = m_units.find(a_fromIdentifier);
		int toUnitId = m_units.find(a_toIdentifier);

		Unit* fromUnit = m_units[fromUnitId];
		Unit* toUnit = m_units[toUnitId];

		if (!fromUnit->hasOutput(a_fromOutputPort))
			return false;
		if (!toUnit->hasInput(a_toInputPort))
			return false;

		// remove record of old connection
		if(toUnit->isConnected(a_toInputPort)) {
			const int nRecords = m_connectionRecords.size();
			for(int i=0;i<nRecords;i++) {
				const ConnectionRecord& rec = m_connectionRecords[i];
				if(rec.to_id == toUnitId && rec.to_port == a_toInputPort) {
					m_connectionRecords.erase(m_connectionRecords.begin() + i);
					break;
				}
			}
		}
		
		// make new connection
		toUnit->connectInput(a_toInputPort, &fromUnit->getOutputValue(a_fromOutputPort));

		// record the connection upon success
		m_connectionRecords.push_back({ fromUnitId, a_fromOutputPort, toUnitId,a_toInputPort });
		_recomputeGraph();
		return true;
	}

	template <typename ID>
	bool Circuit::disconnectInternal(const ID& a_fromIdentifier, int a_fromOutputPort, const ID& a_toIdentifier,
		int a_toInputPort) {
		int fromId = m_units.find(a_fromIdentifier);
		int toId = m_units.find(a_toIdentifier);

		Unit* toUnit = m_units[toId];
		Unit* fromUnit = m_units[fromId];
		_ASSERT(toUnit->getInputSource(a_toInputPort) == &fromUnit->getOutputValue(a_fromOutputPort));

		bool result = toUnit->disconnectInput(a_toInputPort);

		// Find and remove the associated connection record stored in this Circuit
		ConnectionRecord record = { fromId, a_fromOutputPort, toId, a_toInputPort };
		for (unsigned i = 0; i < m_connectionRecords.size(); i++) {
			if (m_connectionRecords[i] == record) {
				result = true;
				m_connectionRecords.erase(m_connectionRecords.begin() + i);
				_recomputeGraph();
				break;
			}
		}
		return result;
	}
};
#endif // __Circuit__
