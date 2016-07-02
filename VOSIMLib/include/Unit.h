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
#include <cereal/types/polymorphic.hpp>
#include <cereal/archives/json.hpp>
#include "NamedContainer.h"
#include "UnitParameter.h"

#define MAX_PARAMS 16
#define MAX_INPUTS 8
#define MAX_OUTPUTS 8

#define DERIVE_UNIT(TYPE) \
	string _getClassName() const override {return #TYPE;}\
	Unit* _clone() const override {return new TYPE(*this);}\
public:\
	TYPE() : TYPE("") {}\
private:

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

	struct UnitPort
	{
		UnitPort() : UnitPort(0.0) {}
		UnitPort(double a_defVal) : defVal(a_defVal), src(nullptr) {}
		
		double defVal;
		const double* src;
	};

	const vector<string> g_bpmStrs = { "4", "7/2", "3", "5/2", "2", "3/2", "1", "3/4", "1/2", "3/8", "1/4", "3/16", "1/8", "3/32", "1/16", "3/64", "1/32", "1/64" };
	const vector<double> g_bpmVals = { 4.0, 7.0 / 2.0, 3.0, 5.0 / 2.0, 2.0, 3.0 / 2.0, 1.0, 3.0 / 4.0, 1.0 / 2.0, 3.0 / 8.0, 1.0 / 4.0, 3.0 / 16.0, 1.0 / 8.0, 3.0 / 32.0, 1.0 / 16.0, 3.0 / 64.0, 1.0 / 32.0, 1.0 / 64.0 };

	/**
	 * \class Unit
	 *
	 * \brief Units encapsulate a discrete processor with an internal state, plus a collection of inputs, outputs, and parameters.
	 *
	 * A unit is composed of internal state variables, a number of outputs and inputs,
	 * and a transition function, Unit::process_, which updates the Unit's outputs given the state
	 * of the Unit and the current inputs + parameters.
	 *
	 * New units should be derived by adding the DERIVE_UNIT macro immediately after the class declaration, and using
	 * Unit::addInput_, Unit::addOutput_, and Unit::addParameter_ to configure the unit in its constructor. 
	 * A constructor taking a single string argument should be supplied. The default constructor is generated automatically.
	 * Lastly, register the unit with cereal using the CEREAL_REGISTER_TYPE macro. This call must be placed in the global namespace.
	 * For example:
	 * \code{.cpp}
	 * namespace mynamespace {
	 *		class DerivedUnit : public Unit {
	 *			DERIVE_UNIT(DerivedUnit)
	 *		public:
	 *			DerivedUnit(const string& a_name) : Unit(a_name) 
	 *			{
	 *			...set up class internals...
	 *			}
	 *		};
	 * }
	 * CEREAL_REGISTER_TYPE(mynamespace::DerivedUnit)
	 * \endcode
	 *
	 * The macros ensure that the code needed to enable serialization and cloning of your class are included:
	 * To allow the unit to be cloned, it is necessary to implement a copy constructor, Unit::_clone, and Unit::_getClassName. Unit::_clone should simply return a
	 * new copy of the derived class, for example:
	 * \code{.cpp}
	 *		Unit* DerivedUnit::_clone(){ return new DerivedUnit(*this); }
	 * \endcode
	 *
	 * Unit::_getClassName should simply return a string form of the class name. This is used for factory construction.
	 * 
	 */
	class Unit
	{
	public:
		Unit(); 

		explicit Unit(const string& a_name);

		virtual ~Unit() { }

		void MSFASTCALL tick() GCCFASTCALL;

		void setFs(double a_newFs);

		void setTempo(double a_newTempo);

		void noteOn(int a_note, int a_velocity);

		void noteOff(int a_note, int a_velocity);

		void notifyParameterChanged(int a_id);

		double getFs() const;

		double getTempo() const;

		bool isNoteOn() const;

		int getNote() const;

		int getVelocity() const;

		//void sendMidiEvent();
		virtual bool isActive() const;

		const Circuit* getParent() const;

		template <typename ID>
		bool hasParameter(const ID& a_paramId) const;

		template <typename ID>
		UnitParameter& getParameter(const ID& a_identifier);

		template <typename ID>
		const UnitParameter& getParameter(const ID& a_identifier) const;

		template <typename ID, typename T>
		bool setParameterValue(const ID& a_identifier, const T& a_value);

		template <typename ID, typename T>
		bool setParameterNorm(const ID& a_identifier, const T& a_value);

		template <typename ID>
		bool setParameterPrecision(const ID& a_identifier, int a_value);

		template <typename ID>
		bool setParameterFromString(const ID& a_identifier, const string& a_value);

		bool hasOutput(int a_outputPort) const;

		bool hasInput(int a_inputPort) const;

		string getInputName(int a_index) const;

		template <typename ID>
		string getOutputName(const ID& a_identifier) const;

		template <typename ID>
		const double& getOutputValue(const ID& a_identifier) const;

		const double& getInputValue(int a_index) const;

		const double* Unit::getInputSource(int a_index) const;

		int getNumParameters() const;

		int getNumInputs() const;

		int getNumOutputs() const;

		/**
		* Connect the specified input port to a location in memory.
		* \param a_toInputPort The desired input port of this unit
		* \param a_src A pointer to the memory to read the input from
		*/
		void connectInput(int a_inputPort, const double* a_src);

		/**
		 * \returns True if the input port points to a non-null location in memory.
		 */
		bool isConnected(int a_inputPort) const;

		/**
		* Remove the specified connection.
		* \returns True if the connection existed, false if it was already disconnected.
		*/
		bool disconnectInput(int a_toInputPort);

		const string& getName() const;

		unsigned int getClassIdentifier() const;

		/**
		 * Copies this unit into newly allocated memory (the caller is responsible for releasing the memory).
		 * Connections to other units are not preserved in the clone.
		 */
		Unit* clone() const; 
		
		template<class Archive>
		void save(Archive& archive) const {
			unsigned int classId = getClassIdentifier();
			archive(
				cereal::make_nvp("class-id", classId),
				cereal::make_nvp("name", m_name),
				cereal::make_nvp("parameters", m_parameters)
			);
		}

		template<class Archive>
		void load(Archive& archive) {
			NamedContainer<UnitParameter, MAX_PARAMS> tmpparams;
			archive(
				cereal::make_nvp("name", m_name),
				cereal::make_nvp("parameters", tmpparams)
			);

			for(int i=0;i<tmpparams.size();i++) {
				int index = tmpparams.getIndices()[i];
				setParameterFromString(index,tmpparams[index].getValueString());
			}
		}

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

		virtual void onInputConnection_(int a_inputPort) { };

		virtual void onInputDisconnection_(int a_inputPort) { };

		template <typename ID>
		void setOutputChannel_(const ID& a_id, const double& a_val);

		virtual void MSFASTCALL process_() GCCFASTCALL = 0;

		int addInput_(const string& a_name, double a_default = 0.0);
		bool addInput_(int a_id, const string& a_name, double a_default = 0.0);

		int addOutput_(const string& a_name);
		bool addOutput_(int a_id, const string& a_name);

		int addParameter_(const UnitParameter& a_param);
		bool addParameter_(int a_id, const UnitParameter& a_param);

	private:
		virtual inline string _getClassName() const = 0;

		void _setParent(Circuit* a_new_parent);

		void _setName(const string& a_name);

		virtual Unit* _clone() const = 0;
	private:
		friend class UnitFactory;
		friend class Circuit;

		string m_name;
		NamedContainer<UnitParameter, MAX_PARAMS> m_parameters;
		NamedContainer<double, MAX_OUTPUTS> m_outputSignals;
		NamedContainer<UnitPort, MAX_INPUTS> m_inputPorts;
		Circuit* m_parent;
		AudioConfig m_audioConfig;
		MidiData m_midiData;
	};

	template <typename ID>
	const double& Unit::getOutputValue(const ID& a_identifier) const {
		return m_outputSignals[a_identifier];
	}

	template <typename ID>
	UnitParameter& Unit::getParameter(const ID& a_identifier) {
		return m_parameters[a_identifier];
	}

	template <typename ID>
	const UnitParameter& Unit::getParameter(const ID& a_identifier) const {
		return m_parameters[a_identifier];
	}

	template <typename ID>
	void Unit::setOutputChannel_(const ID& a_id, const double& a_val)
	{
		m_outputSignals[a_id] = a_val;
	}

	template <typename ID>
	bool Unit::hasParameter(const ID& a_paramId) const
	{
		return m_parameters.contains(a_paramId);
	}

	template <typename ID, typename T>
	bool Unit::setParameterValue(const ID& a_identifier, const T& a_value) {
		return m_parameters[a_identifier].set(a_value);
	};

	template <typename ID, typename T>
	bool Unit::setParameterNorm(const ID& a_identifier, const T& a_value) {
		return m_parameters[a_identifier].setNorm(a_value);
	}

	template <typename ID>
	bool Unit::setParameterPrecision(const ID& a_identifier, int a_precision) {
		return m_parameters[a_identifier].setPrecision(a_precision);
	}

	template <typename ID>
	bool Unit::setParameterFromString(const ID& a_identifier, const string& a_value) {
		return m_parameters[a_identifier].setFromString(a_value);
	}

	template <typename ID>
	string Unit::getOutputName(const ID& a_identifier) const {
		return m_outputSignals.getItemName<ID>(a_identifier);
	};
}
#endif
