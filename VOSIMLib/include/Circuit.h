#ifndef __Circuit__
#define __Circuit__

#include "Unit.h"
#include "NamedContainer.h"
#include <vector>
#include <memory>

using std::shared_ptr;
using std::vector;

namespace syn {

    struct ConnectionRecord {
        enum {
            Null = 0,
            Internal,
            Input,
            Output
        } type;
        int from_id;
        int from_port;
        int to_id;
        int to_port;

        bool operator==(const ConnectionRecord& a_other) const
        {
            bool res = (type == a_other.type) && ( from_port == a_other.from_port ) && (to_id == a_other.to_id && to_port == a_other.to_port);
            if(type==Internal){
                res = res && (from_id == a_other.from_id);
            }
            return res;
        }
    };

    /**
    * \class Circuit
    *
    * \brief A collection of Units.
    *
    * A Circuit is a Unit that contains other Units.
    *
    */
    class Circuit : public Unit {
    public:
        Circuit();

        Circuit(const string& a_name);

        Circuit(const Circuit& a_other);

        /**
         * \returns True if any of its internal units are active
         */
        virtual bool isActive() const;

        const Unit& getUnit(int a_unitId) const;

        template<typename UID>
        int getUnitId(const UID& a_unitIdentifier) const;

        int getNumUnits() const;

        /**
         * Retrieves a list of output ports connected to an input port.
         * \returns A vector of pairs of the form {unit_id, port_id}
         */
        template<typename UID>
        vector<pair<int, int> > getConnectionsToInternalInput(const UID& a_unitIdentifier, int a_portid) const;

        vector<pair<int, int> > getConnectionsToCircuitInput(int a_circuitInputPort) const;

        vector<pair<int, int> > getConnectionsToCircuitOutput(int a_circuitOutputPort) const;

		const vector<ConnectionRecord>& getConnectionRecords() const;

        template<typename UID, typename PID>
        double getInternalParameter(const UID& a_unitIdentifier, const PID& a_paramIdentifier) const;

        template<typename UID, typename PID, typename T>
        bool setInternalParameter(const UID& a_unitIdentifier, const PID& a_paramIdentifier, const T& a_value);

        template<typename UID, typename PID, typename T>
        bool setInternalParameterNorm(const UID& a_unitIdentifier, const PID& a_paramIdentifier, const T& a_value);

        /**
         * \returns The id assigned to the newly added unit, or -1 on failure
         */
        int addUnit(shared_ptr<Unit> a_unit);

        template<typename ID>
        bool removeUnit(const ID& a_identifier);

        template<typename ID>
        bool connectInputs(int a_circuitInputPort, const ID& a_toIdentifier, int a_toInputPort);

        template<typename ID>
        bool connectOutputs(int a_circuitOutputPort, const ID& a_fromIdentifier, int a_fromOutputPort);

        template<typename ID>
        bool connectInternal(const ID& a_fromIdentifier, int a_fromOutputPort, const ID& a_toIdentifier, int
        a_toInputPort);

        template<typename ID>
        bool disconnectInputs(int a_circuitInputPort, const ID& a_toIdentifier, int a_toInputPort);

        template<typename ID>
        bool disconnectOutputs(int a_circuitOutputPort, const ID& a_fromIdentifier, int a_fromOutputPort);

        template<typename ID>
        bool disconnectInternal(const ID& a_fromIdentifier, int a_fromOutputPort, const ID& a_toIdentifier, int
        a_toInputPort);

    protected:
	    void process_(const SignalBus& a_inputs, SignalBus& a_outputs) override;

	    void onFsChange_() override;

	    void onTempoChange_() override;

	    void onNoteOn_() override;

	    void onNoteOff_() override;

    private:
	    string _getClassName() const override {
            return "Circuit";
        };

	    Unit* _clone() const override;

    private:
		friend class VoiceManager;
        NamedContainer<shared_ptr<Unit> > m_units;
        UnitConnectionBus m_externalInputs;
        UnitConnectionBus m_externalOutputs;
        vector<ConnectionRecord> m_connectionRecords;
    };

    template<typename UID>
    vector<pair<int, int> > Circuit::getConnectionsToInternalInput(const UID& a_unitIdentifier, int a_portid) const
    {
        int unitId = m_units.getItemIndex(a_unitIdentifier);
        vector<pair<int, int> > connectedPorts;
        for (int i = 0 ; i < m_connectionRecords.size() ; i++) {
            const ConnectionRecord& conn = m_connectionRecords[i];
            if (conn.type == ConnectionRecord::Internal && conn.to_id == unitId && conn.to_port == a_portid) {
                connectedPorts.push_back(make_pair(conn.from_id, conn.from_port));
            }
        }
        return connectedPorts;
    }

    template<typename UID>
    int Circuit::getUnitId(const UID& a_unitIdentifier) const
    {
        return m_units.getItemIndex(a_unitIdentifier);
    }

    template<typename UID, typename PID, typename T>
    bool Circuit::setInternalParameter(const UID& a_unitIdentifier, const PID& a_paramIdentifier, const T& a_value)
    {
        if (!m_units.find(a_unitIdentifier))
            return false;
        return m_units[a_unitIdentifier]->setParameter(a_paramIdentifier, a_value);
    };

    template<typename UID, typename PID, typename T>
    bool Circuit::setInternalParameterNorm(const UID& a_unitIdentifier, const PID& a_paramIdentifier, const T& a_value)
    {
        if (!m_units.find(a_unitIdentifier))
            return false;
        return m_units[a_unitIdentifier]->setParameterNorm(a_paramIdentifier, a_value);
    };


    template<typename UID, typename PID>
    double Circuit::getInternalParameter(const UID& a_unitIdentifier, const PID& a_paramIdentifier) const
    {
        if (!m_units.find(a_unitIdentifier))
            return false;
        return m_units[a_unitIdentifier]->getParameter(a_paramIdentifier).getDouble();
    };

    template<typename ID>
    bool Circuit::removeUnit(const ID& a_unitIdentifier)
    {
		if (!m_units.find(a_unitIdentifier))
			return false;
        shared_ptr<Unit> unit = m_units[a_unitIdentifier];
        int unitId = m_units.getItemIndex(a_unitIdentifier);
        // Erase connections
        for (int i = 0 ; i < m_connectionRecords.size() ; i++) {
            const ConnectionRecord& rec = m_connectionRecords[i];
			switch(rec.type) {
			case ConnectionRecord::Internal: 
				if (unitId == rec.to_id || unitId == rec.from_id) {
					disconnectInternal(rec.from_id, rec.from_port, rec.to_id, rec.to_port);
					// disconnect methods remove the connection record from the list, so we need to decrement our index to
					// compensate
					i--;
				}else {
					if (rec.to_id > unitId) { // drop unit id's to compensate for the removed unit
						m_connectionRecords[i].to_id -= 1;
					}
					if (rec.from_id > unitId) { // drop unit id's to compensate for the removed unit
						m_connectionRecords[i].from_id -= 1;
					}
				}
				break;
			case ConnectionRecord::Input:
				if (unitId == rec.to_id) {
					disconnectInputs(rec.from_port, rec.to_id, rec.to_port);
					i--;
				}
				else if (rec.to_id > unitId) {
					m_connectionRecords[i].to_id -= 1;
				}
				break;
			case ConnectionRecord::Output:
				if (unitId == rec.to_id) {
					disconnectOutputs(rec.from_port, rec.to_id, rec.to_port);
					i--;
				}else if(rec.to_id > unitId) {
					m_connectionRecords[i].to_id -= 1;
				}
				break;
			case ConnectionRecord::Null:
			default: break;
			}
        }
        m_units.remove(a_unitIdentifier);
	    return true;
    }

    template<typename ID>
    bool Circuit::connectInputs(int a_circuitInputPort, const ID& a_toIdentifier, int a_toInputPort)
    {
        int toUnitId = m_units.getItemIndex(a_toIdentifier);
        if (!m_units.find(toUnitId))
            return false;

        shared_ptr<Unit> toUnit = m_units[toUnitId];

        if (toUnit->getNumInputs() <= a_toInputPort || getNumInputs() <= a_circuitInputPort)
            return false;

        bool result = m_externalInputs.connect(toUnit, a_toInputPort, a_circuitInputPort);
        // Record connection upon success
        if (result) {
            m_connectionRecords.push_back({ConnectionRecord::Input, 0, a_circuitInputPort, toUnitId, a_toInputPort});
        }
        return result;
    }

    template<typename ID>
    bool Circuit::connectOutputs(int a_circuitOutputPort, const ID& a_fromIdentifier, int a_fromOutputPort)
    {
        int fromUnitId = m_units.getItemIndex(a_fromIdentifier);
        if (!m_units.find(fromUnitId))
            return false;

        shared_ptr<Unit> fromUnit = m_units[fromUnitId];

        int nFromOutputs = fromUnit->getNumOutputs();
        int nCircOutputs = getNumOutputs();
        if (nFromOutputs <= a_fromOutputPort || nCircOutputs <= a_circuitOutputPort)
            return false;

        bool result = m_externalOutputs.connect(fromUnit, a_fromOutputPort, a_circuitOutputPort);
        if (result) { // Record connection upon success
            m_connectionRecords.push_back({ConnectionRecord::Output, 0, a_circuitOutputPort, fromUnitId, a_fromOutputPort});
        }
        return result;
    }

    template<typename ID>
    bool Circuit::connectInternal(const ID& a_fromIdentifier, int a_fromOutputPort, const ID& a_toIdentifier,
                                  int a_toInputPort)
    {
        int fromUnitId = m_units.getItemIndex(a_fromIdentifier);
        int toUnitId = m_units.getItemIndex(a_toIdentifier);
        if (!m_units.find(fromUnitId) || !m_units.find(toUnitId))
            return false;

        shared_ptr<Unit> fromUnit = m_units[fromUnitId];
        shared_ptr<Unit> toUnit = m_units[toUnitId];

        if (fromUnit->getNumOutputs() <= a_fromOutputPort || toUnit->getNumInputs() <= a_toInputPort)
            return false;

        bool result = toUnit->_connectInput(fromUnit, a_fromOutputPort, a_toInputPort);
        // record the connection upon success
        if (result) {
            m_connectionRecords.push_back({ConnectionRecord::Internal, fromUnitId, a_fromOutputPort, toUnitId,
                                           a_toInputPort});
        }
	    return result;
    }

    template<typename ID>
    bool Circuit::disconnectInputs(int a_circuitInputPort, const ID& a_toIdentifier, int a_toInputPort)
    {
        int toUnitId = m_units.getItemIndex(a_toIdentifier);
        if (!m_units.find(toUnitId))
            return false;

        shared_ptr<Unit> toUnit = m_units[toUnitId];

        if (toUnit->getNumInputs() <= a_toInputPort || getNumInputs() <= a_circuitInputPort)
            return false;

        bool result = m_externalInputs.disconnect(toUnit, a_toInputPort, a_circuitInputPort);
        // find and remove the associated connection record upon successful removal
        if (result) {
            ConnectionRecord record = {ConnectionRecord::Input, 0, a_circuitInputPort, toUnitId, a_toInputPort};

            for (unsigned i = 0 ; i < m_connectionRecords.size() ; i++) {
                if (m_connectionRecords[i] == record) {
                    m_connectionRecords.erase(m_connectionRecords.begin() + i);
                    break;
                }
            }
        }
        return result;
    }

    template<typename ID>
    bool Circuit::disconnectOutputs(int a_circuitOutputPort, const ID& a_fromIdentifier, int a_fromOutputPort)
    {
        int fromUnitId = m_units.getItemIndex(a_fromIdentifier);
        if (!m_units.find(fromUnitId))
            return false;

        shared_ptr<Unit> fromUnit = m_units[fromUnitId];

        if (fromUnit->getNumOutputs() <= a_fromOutputPort || getNumOutputs() <= a_circuitOutputPort)
            return false;

        bool result = m_externalOutputs.disconnect(fromUnit, a_fromOutputPort, a_circuitOutputPort);
        // find and remove the associated connection record upon successful removal
        if (result) {
            ConnectionRecord record = {ConnectionRecord::Output, 0, a_circuitOutputPort, fromUnitId, a_fromOutputPort};
            for (unsigned i = 0 ; i < m_connectionRecords.size() ; i++) {
                if (m_connectionRecords[i] == record) {
                    m_connectionRecords.erase(m_connectionRecords.begin() + i);
                    break;
                }
            }
        }
        return result;
    }

    template<typename ID>
    bool Circuit::disconnectInternal(const ID& a_fromIdentifier, int a_fromOutputPort, const ID& a_toIdentifier,
                                     int a_toInputPort)
    {
        int fromId = m_units.getItemIndex(a_fromIdentifier);
        int toId = m_units.getItemIndex(a_toIdentifier);
        if (!m_units.find(fromId) || !m_units.find(toId))
            return false;

        shared_ptr<Unit> fromUnit = m_units[fromId];
        shared_ptr<Unit> toUnit = m_units[toId];

        if (fromUnit->getNumOutputs() <= a_fromOutputPort || toUnit->getNumInputs() <= a_toInputPort)
            return false;

        bool result = toUnit->_disconnectInput(fromUnit, a_fromOutputPort, a_toInputPort);
        // find and remove the associated connection record upon successful removal
        if (result) {
            ConnectionRecord record = {ConnectionRecord::Internal, fromId, a_fromOutputPort, toId,
                                       a_toInputPort};
            for (unsigned i = 0 ; i < m_connectionRecords.size() ; i++) {
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
