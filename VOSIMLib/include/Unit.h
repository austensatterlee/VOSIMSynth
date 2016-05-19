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

#ifndef __UNIT__
#define __UNIT__

#include "SignalBus.h"
#include "NamedContainer.h"
#include "UnitConnectionBus.h"
#include "UnitParameter.h"
#include <memory>

using std::shared_ptr;
using std::make_shared;

namespace syn
{
	class Circuit;

	struct AudioConfig
	{
		double fs;
		double tempo;
	};

	struct MidiData
	{
		int note;
		int velocity;
		bool isNoteOn;
	};

	const vector<string> g_bpmStrs = { "1/64", "1/32", "3/64", "1/16", "3/32", "1/8", "3/16", "1/4", "3/8", "1/2", "3/4", "1", "3/2", "2" };
	const vector<double> g_bpmVals = { 1.0 / 64.0, 1.0 / 32.0, 3.0 / 64.0, 1.0 / 16.0, 3.0 / 32.0, 1.0 / 8.0, 3.0 / 16.0, 1.0 / 4.0, 3.0 / 8.0, 1.0 / 2.0, 3.0 / 4.0, 1.0, 3.0 / 2.0, 2.0 };

	/**
	 * \class Unit
	 *
	 * \brief Units encapsulate a discrete processor with an internal state, plus a collection of inputs, outputs, and parameters.
	 *
	 * A unit is composed of internal state variables, a number of outputs and inputs,
	 * and a transition function, Unit::process_, which updates the Unit's outputs given the state
	 * of the Unit and the current inputs + parameters.
	 *
	 * New units should be derived by subclassing Unit, using Unit::addInput_, Unit::addOutput_, and Unit::addParameter_ to configure the unit.
	 * To allow the unit to be cloned, it is necessary to implement a copy constructor,
	 * Unit::_clone, and Unit::_getClassName. Unit::_clone should simply return a new copy of the derived class, for example:
	 *
	 *		Unit* DerivedUnit::_clone(){ return new DerivedUnit(*this); }
	 *
	 * Unit::_getClassName should simply return a string form of the class name. This is used for serialization.
	 *
	 */
	class Unit
	{
	public:
		Unit();

		explicit Unit(const string& a_name);

		virtual ~Unit() { };

		void MSFASTCALL tick() GCCFASTCALL;

		void MSFASTCALL reset() GCCFASTCALL;

		void setFs(double a_newFs);

		void setTempo(double a_newTempo);

		void noteOn(int a_note, int a_velocity);

		void noteOff(int a_note, int a_velocity);

		double getFs() const;

		double getTempo() const;

		bool isNoteOn() const;

		int getNote() const;

		int getVelocity() const;

		//void sendMidiEvent();
		virtual bool isActive() const;

		const Circuit* getParent() const;

		template <typename ID>
		const UnitParameter& getParameter(const ID& a_identifier) const;

		template <typename ID, typename T>
		bool setParameter(const ID& a_identifier, const T& a_value);

		template <typename ID, typename T>
		bool setParameterNorm(const ID& a_identifier, const T& a_value);

		template <typename ID>
		bool setParameterPrecision(const ID& a_identifier, int a_value);

		template <typename ID>
		bool setParameterFromString(const ID& a_identifier, const string& a_value);

		template <typename ID>
		string getInputName(const ID& a_identifier) const;

		template <typename ID>
		string getOutputName(const ID& a_identifier) const;

		template <typename ID>
		const Signal& getOutputChannel(const ID& a_identifier) const;

		template <typename ID>
		const Signal& getInputChannel(const ID& a_identifier) const;

		template <typename ID, typename T>
		bool setInputChannel(const ID& a_identifier, const T& a_val);

		int getNumParameters() const;

		int getNumInputs() const;

		int getNumOutputs() const;

		const string& getName() const;

		unsigned int getClassIdentifier() const;

		/**
		 * Copies this unit into newly allocated memory (the caller is responsible for releasing the memory).
		 * Connections to other units are not preserved in the clone.
		 */
		Unit* clone() const;
	protected:
		/**
		 * Called when a parameter has been modified. This function should be overridden
		 * to update the internal state and verify that the change is valid. If the change is not valid,
		 * the function should return false.
		 */
		virtual void onParamChange_(int a_paramId) { };

		virtual void onFsChange_() { };

		virtual void onTempoChange_() { };

		virtual void onNoteOn_() { };

		virtual void onNoteOff_() { };

		virtual void onMidiControlChange_(int a_cc, double a_value) { };

		template <typename ID>
		UnitParameter& getParameter_(const ID& a_identifier);

		virtual void MSFASTCALL process_(const SignalBus& a_inputs, SignalBus& a_outputs) GCCFASTCALL = 0;

		int addInput_(const string& a_name, double a_default = 0.0, Signal::ChannelAccType a_accType = Signal::ChannelAccType::EAdd);

		int addOutput_(const string& a_name);

		int addParameter_(const UnitParameter& a_param);

	private:
		virtual inline string _getClassName() const = 0;

		void _setParent(Circuit* a_new_parent);

		void _setName(const string& a_name);

		/**
		 * Connect the specified output port to this units specified input port.
		 * \param a_fromUnit The output unit
		 * \param a_fromOutputPort The output port of a_fromUnit
		 * \param a_toInputPort The input port of this unit
		 * \returns False if the connection already existed
		 */
		bool _connectInput(shared_ptr<Unit> a_fromUnit, int a_fromOutputPort, int a_toInputPort);

		/**
		 * Remove the specified connection
		 * \returns True if the connection existed
		 */
		bool _disconnectInput(shared_ptr<Unit> a_fromUnit, int a_fromOutputPort, int a_toInputPort);

		/**
		 * Remove all connections referencing the specified Unit
		 * \returns True if any connections were removed
		 */
		bool _disconnectInput(shared_ptr<Unit> a_fromUnit);

		virtual Unit* _clone() const = 0;
	private:
		friend class Circuit;
		friend class UnitFactory;
		friend class VoiceManager;
		friend class UnitConnectionBus;
		string m_name;
		bool m_hasTicked;
		NamedContainer<UnitParameter> m_parameters;
		SignalBus m_inputSignals;
		SignalBus m_outputSignals;
		UnitConnectionBus m_inputConnections;
		Circuit* m_parent;
		AudioConfig m_audioConfig;
		MidiData m_midiData;
	};


	template <typename ID>
	const Signal& Unit::getOutputChannel(const ID& a_identifier) const {
		return m_outputSignals.getChannel(a_identifier);
	}

	template <typename ID>
	const Signal& Unit::getInputChannel(const ID& a_identifier) const {
		return m_inputSignals.getChannel(a_identifier);
	}

	template <typename ID, typename T>
	bool Unit::setInputChannel(const ID& a_identifier, const T& a_val) {
		return m_inputSignals.setChannel(a_identifier, a_val);
	}

	template <typename ID>
	UnitParameter& Unit::getParameter_(const ID& a_identifier) {
		return m_parameters[a_identifier];
	};

	template <typename ID>
	const UnitParameter& Unit::getParameter(const ID& a_identifier) const {
		return m_parameters[a_identifier];
	}

	template <typename ID, typename T>
	bool Unit::setParameter(const ID& a_identifier, const T& a_value) {		
		if (m_parameters[a_identifier].set(a_value)) {
			int id = m_parameters.getItemId(a_identifier);
			onParamChange_(id);
			return true;
		}
		return false;
	};

	template <typename ID, typename T>
	bool Unit::setParameterNorm(const ID& a_identifier, const T& a_value) {
		if (m_parameters[a_identifier].setNorm(a_value)) {
			int id = m_parameters.getItemId(a_identifier);
			onParamChange_(id);
			return true;
		}
		return false;
	}

	template <typename ID>
	bool Unit::setParameterPrecision(const ID& a_identifier, int a_precision) {
		m_parameters[a_identifier].setPrecision(a_precision);
		return true;
	}

	template <typename ID>
	bool Unit::setParameterFromString(const ID& a_identifier, const string& a_value) {
		m_parameters[a_identifier].setFromString(a_value);
		return true;
	}

	template <typename ID>
	string Unit::getInputName(const ID& a_identifier) const {
		return m_inputSignals.getChannelName<ID>(a_identifier);
	}

	template <typename ID>
	string Unit::getOutputName(const ID& a_identifier) const {
		return m_outputSignals.getChannelName<ID>(a_identifier);
	};
}
#endif
