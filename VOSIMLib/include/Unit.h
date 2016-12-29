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

/**
 *  \file Unit.h
 *  \brief
 *  \details
 *  \author Austen Satterlee
 *  \date 02/2016
 */

#ifndef __UNIT__
#define __UNIT__

#include "common_serial.h"
#include "common.h"
#include "NamedContainer.h"
#include "UnitParameter.h"
#include "UnitFactory.h"

#include <eigen/Core>

#define MAX_PARAMS 16
#define MAX_INPUTS 8
#define MAX_OUTPUTS 8

#define DERIVE_UNIT(TYPE) \
    Unit *_clone() const override {return new TYPE(*this);}\
public:\
    string getClassName() const override {return #TYPE;}\
    TYPE() : TYPE("") {}\
private:

namespace syn
{
    class Circuit;

    struct VOSIMLIB_API AudioConfig
    {
        double fs;
        double tempo;
    };

    struct VOSIMLIB_API MidiData
    {
        int note;
        int velocity;
        bool isNoteOn;
    };

    struct VOSIMLIB_API UnitPort
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
     * A unit is composed of internal state variables, a number of outputs and inputs, and a transition
     * function, Unit::process_, which updates the Unit's outputs given the state of the Unit and the current
     * inputs + parameters.
     *
     * New units should be derived by adding the DERIVE_UNIT macro immediately after the class declaration,
     * and using Unit::addInput_, Unit::addOutput_, and Unit::addParameter_ to configure the unit in its
     * constructor.  A constructor taking a single string argument should be supplied. The default constructor
     * is generated automatically.
     *
     * Serialization is done via a conversion operator to the nlohmann::json type. This can be overriden to
     * specialize serialization, but make sure to call the base class version first since this takes care of
     * serializing things like the unit's name and parameters.  Deserialization happens via the static
     * function Unit::fromJSON, which creates a new Unit object using the UnitFactory singleton. However, the
     * virtual function Unit::load can be overridden to provide any specialized deserialization.
     *
     * For example:
     * \code{.cpp}
     *  class DerivedUnit : public Unit {
     *        DERIVE_UNIT(DerivedUnit)
     *  public:
     *        DerivedUnit(const string& a_name) : Unit(a_name)
     *        {
     *        ...set up class internals...
     *        }
     *
     *        /// OPTIONAL:
     *        operator json() const override {
     *          json j = Unit::operator json();
     *          ...serialize class internals...
     *          return j;
     *        }
     *
     *        Unit* load(const json& j) override {
     *          ...deserialize class internals...
     *          return this;
     *        }
     *
     *    };
     * \endcode
     *
     * The macros ensure that the code needed to enable serialization and cloning of your class are included:
     * To allow the unit to be cloned, it is necessary to implement a copy constructor, Unit::_clone, and
     * Unit::getClassName. Unit::_clone should simply return a new copy of the derived class, for example:
     * \code{.cpp}
     *        Unit* DerivedUnit::_clone(){ return new DerivedUnit(*this); }
     * \endcode
     *
     * Unit::getClassName should simply return a string form of the class name. This is used for factory
     * construction.
     *
     */
    class VOSIMLIB_API Unit
    {
    public:
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW

            Unit();

        explicit Unit(const string& a_name);

        virtual ~Unit() { }

        /**
         * Clears the unit outputs and calls the unit's process_ method.
         */
        void MSFASTCALL tick() GCCFASTCALL;

        /**
         * Notify the unit of a sample rate change
         */
        void setFs(double a_newFs);

        /**
         * Notify the unit of a tempo change
         */
        void setTempo(double a_newTempo);

        /**
         * Notify the unit of a note on event
         */
        void noteOn(int a_note, int a_velocity);

        /**
         * Notify the unit of a note off event
         */
        void noteOff(int a_note, int a_velocity);

        /**
         * Reset the units internal state (if any)
         */
        virtual void reset() {};

        /**
         * Notify the unit that one of its internal parameters changed.
         * This is automatically called by the internal UnitParameter.
         */
        void notifyParameterChanged(int a_id);

        double fs() const;

        double tempo() const;

        bool isNoteOn() const;

        int note() const;

        int velocity() const;

        /**
         * This method is used to determine the lifespan of a voice.
         * By default, this method returns true when a note is on and false otherwise.
         * It should be overriden to suite the unit's specific needs.
         */
        virtual bool isActive() const;

        /**
         * Returns a pointer to the unit's parent Circuit, or nullptr if there is none.
         */
        const Circuit* parent() const;

        /**
         * Determines whether or not the unit contains a specific parameter.
         */
        template <typename ID>
        bool hasParam(const ID& a_paramId) const;

        /**
         * Returns a reference to the requested parameter.
         */
        template <typename ID>
        UnitParameter& param(const ID& a_identifier);

        template <typename ID>
        const UnitParameter& param(const ID& a_identifier) const;

        const string& paramName(int a_index) const;

        const NamedContainer<UnitParameter, MAX_PARAMS>& parameters() const;

        /**
         * Sets the value of an internal parameter
         * \see UnitParameter::set
         */
        template <typename ID, typename T>
        bool setParam(const ID& a_identifier, const T& a_value);

        bool hasInput(int a_inputPort) const;

        const string& inputName(int a_index) const;

        const double& readInput(int a_index) const;

        const double* inputSource(int a_index) const;

        const NamedContainer<UnitPort, MAX_INPUTS>& inputs() const;

        bool hasOutput(int a_outputPort) const;

        template <typename ID>
        string outputName(const ID& a_identifier) const;

        template <typename ID>
        const double& readOutput(const ID& a_identifier) const;

        const NamedContainer<double, MAX_OUTPUTS>& outputs() const;

        int numParams() const;

        int numInputs() const;

        int numOutputs() const;

        /**
        * Connect the specified input port to a location in memory.
        * \param a_inputPort The desired input port of this unit
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

        const string& name() const;
        void setName(const string& a_name);

        unsigned int getClassIdentifier() const;

        /**
         * Copies this unit into newly allocated memory (the caller is responsible for releasing the memory).
         * Connections to other units are not preserved in the clone.
         */
        Unit* clone() const;

        /// Construct a new unit from a json description
        static Unit* fromJSON(const json& j);

        /// Serialize to json
        virtual operator json() const;

        /// Optional method that can be overriden to load extra information.
        virtual Unit* load(const json& j) { return this; }

        /// Returns a unique string to identify the class
        virtual inline string getClassName() const = 0;

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

        void copyFrom_(const Unit& a_other);

    private:
        void _setParent(Circuit* a_new_parent);

        void _setName(const string& a_name);

        virtual Unit* _clone() const = 0;

    private:
        friend class UnitFactory;
        friend class Circuit;
    public:
        typedef ::Eigen::Array<double, -1, -1, Eigen::RowMajor> dynamic_buffer_t;

        void MSFASTCALL tick(const dynamic_buffer_t& a_inputs, dynamic_buffer_t& a_outputs) GCCFASTCALL;

    private:
        string m_name;
        NamedContainer<UnitParameter, MAX_PARAMS> m_parameters;
        NamedContainer<double, MAX_OUTPUTS> m_outputSignals;
        NamedContainer<UnitPort, MAX_INPUTS> m_inputPorts;
        Circuit* m_parent;
        AudioConfig m_audioConfig;
        MidiData m_midiData;
    };

    template <typename ID>
    const double& Unit::readOutput(const ID& a_identifier) const { return m_outputSignals[a_identifier]; }

    template <typename ID>
    UnitParameter& Unit::param(const ID& a_identifier) { return m_parameters[a_identifier]; }

    template <typename ID>
    const UnitParameter& Unit::param(const ID& a_identifier) const { return m_parameters[a_identifier]; }

    template <typename ID>
    void Unit::setOutputChannel_(const ID& a_id, const double& a_val) { m_outputSignals[a_id] = a_val; }

    template <typename ID>
    bool Unit::hasParam(const ID& a_paramId) const { return m_parameters.contains(a_paramId); }

    template <typename ID, typename T>
    bool Unit::setParam(const ID& a_identifier, const T& a_value) { return m_parameters[a_identifier].set(a_value); };

    template <typename ID>
    string Unit::outputName(const ID& a_identifier) const { return m_outputSignals.name<ID>(a_identifier); };
}
#endif