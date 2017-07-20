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
#include "vosimlib/common_serial.h"
#include "vosimlib/common.h"
#include "vosimlib/StrMap.h"
#include "vosimlib/UnitParameter.h"
#include "vosimlib/UnitFactory.h"

#include <eigen/Core>

#define MAX_PARAMS 16
#define MAX_INPUTS 8
#define MAX_OUTPUTS 8

#define DERIVE_UNIT(TYPE) \
    Unit *_clone() const override {return new TYPE(*this);} \
public: \
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW \
    const string& getClassName() const override {return TYPE::className();} \
    static const string& className() { static const std::string class_name = #TYPE; return class_name; } \
    static const syn::UnitTypeId& classIdentifier() { static const std::hash<string> hash_fn; static const syn::UnitTypeId id = static_cast<syn::UnitTypeId>(hash_fn(#TYPE)); return id; } \
    TYPE() : TYPE("") {} \
private:

#define BEGIN_PROC_FUNC \
    assert(m_currentBufferOffset==0 || m_currentBufferOffset==getBufferSize()); \
    for(m_currentBufferOffset=0;m_currentBufferOffset<getBufferSize();m_currentBufferOffset++){
#define END_PROC_FUNC }
#define READ_OUTPUT(OUTPUT) \
    readOutput(OUTPUT, m_currentBufferOffset)
#define READ_INPUT(INPUT) \
    readInput(INPUT, m_currentBufferOffset)
#define WRITE_OUTPUT(OUTPUT, VALUE) \
    writeOutput_(OUTPUT, m_currentBufferOffset, VALUE)

namespace syn
{
    class Circuit;

    struct VOSIMLIB_API AudioConfig
    {
        double fs;
        double tempo;
        int bufferSize;
    };

    struct VOSIMLIB_API MidiData
    {
        int note;
        int velocity;
        bool isNoteOn;
    };

    struct VOSIMLIB_API InputPort
    {
        InputPort() : InputPort(0.0) {}

        InputPort(double a_defVal) : defVal(a_defVal), src(nullptr) {}

        double defVal;
        const double* src;
    };

    typedef std::vector<double> OutputPort;

    const vector<string> g_bpmStrs = {"4", "7/2", "3", "5/2", "2", "3/2", "1", "3/4", "1/2", "3/8", "1/4", "3/16", "1/8", "3/32", "1/16", "3/64", "1/32", "1/64"};
    const vector<double> g_bpmVals = {4.0, 7.0 / 2.0, 3.0, 5.0 / 2.0, 2.0, 3.0 / 2.0, 1.0, 3.0 / 4.0, 1.0 / 2.0, 3.0 / 8.0, 1.0 / 4.0, 3.0 / 16.0, 1.0 / 8.0, 3.0 / 32.0, 1.0 / 16.0, 3.0 / 64.0, 1.0 / 32.0, 1.0 / 64.0};

    /**
     * \class Unit
     *
     * \brief Basic DSP block with an internal state, plus a collection of I/O ports and user parameters.
     *
     * A unit is composed of internal state variables, a number of input and output ports, and a processing
     * function, Unit::process_, which updates the Unit's output buffers.
     *
     * ## Using the Unit class
     * New units should be derived by adding the `DERIVE_UNIT` macro immediately after the opening brace of the
     * class declaration (see below for an example).
     *
     * A constructor taking a single string argument as the unit's name should be supplied. Unit::addInput_,
     * Unit::addOutput_, and Unit::addParameter_ should be used in the constructor to add input ports, output
     * ports, and user parameters respectively.
     *
     * A copy constructor must also be provided. See the example below for a minimum working copy constructor.
     *
     * ### Serialization
     * The Unit class uses the "JSON For Modern C++" library for serialization.
     *
     * Serialization occurs in the nlohmann::json conversion operator. You may override this function, but
     * make sure to call the base class implementation at some point. This ensures critical information
     * necessary for deserialization (e.g. unit type) is saved.
     * 
     * Deserialization happens via the static method Unit::fromJSON, which leverages the UnitFactory singleton
     * to produce a new instance of your derived class (if it has been registered). See the documentation for
     * UnitFactory to learn how to register your derived type. 
     *
     * To specialize deserialization, override the virtual method Unit::load. This method is called from
     * Unit::fromJSON on the newly created instance.
     *
     * ### Example:
     * \code{.cpp}
     *  class DerivedUnit : public Unit {
     *        DERIVE_UNIT(DerivedUnit)
     *  public:
     *        DerivedUnit(const string& a_name) : Unit(a_name)
     *        {
     *          // ...set up class internals... 
     *        }
     *
     *        // Bare minimum copy constructor needed for cloning to work correctly.
     *        DerivedUnit(const DerivedUnit& a_other) : DerivedUnit(a_other.name()) 
     *        {}
     *
     *        // Optional methods (can be used to implement special behavior)
     *        operator json() const override {
     *          json j = Unit::operator json();
     *          // ...serialize class internals...
     *          return j;
     *        }
     *
     *        Unit* load(const json& j) override {
     *          // ...deserialize class internals...
     *          return this;
     *        }
     *        
     *  protected:
     *        void MSFASTCALL process_() override {
     *          BEGIN_PROC_FUNC
     *          // ...compute output sample...
     *          END_PROC_FUNC
     *        }
     *    };
     * \endcode
     *
     */
    class VOSIMLIB_API Unit
    {
    public:
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    public:
        typedef Eigen::Array<double, -1, -1, Eigen::RowMajor> dynamic_buffer_t;

        Unit();

        explicit Unit(const string& a_name);

        virtual ~Unit() {}

        /**
         * Processes as many samples as required to fill the internal buffer (see Unit::getBufferSize and Unit::setBufferSize).
         */
        inline void MSFASTCALL tick() GCCFASTCALL;

        /**
         * Processes as many samples to fill the specified output buffer. 
         * 
         * Note that both buffers must have the same shape. Each row of a buffer corresponds to one of the
         * Unit's input/output ports, in order by port id. That is, the first row corresponds to the port with
         * the lowest id, the next row the port with the second lowest, etc...
         *
         * \param a_inputs Input buffer.
         * \param a_outputs Output buffer.
         */
        void MSFASTCALL tick(const dynamic_buffer_t& a_inputs, dynamic_buffer_t& a_outputs) GCCFASTCALL;

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
         * Reset the unit's internal state (if any), as if Unit::tick had never been called.
         */
        virtual void reset() = 0;

        virtual void setBufferSize(int a_bufferSize);

        int getBufferSize() const;

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
        bool hasParam(int a_id) const { return m_parameters.containsId(a_id); }
        bool hasParam(const std::string& a_name) const { return m_parameters.containsName(a_name); }

        /**
         * Returns a reference to the requested parameter.
         */
        template <typename ID>
        UnitParameter& param(const ID& a_id);

        template <typename ID>
        const UnitParameter& param(const ID& a_id) const;

        const string& paramName(int a_id) const;

        const StrMap<UnitParameter, MAX_PARAMS>& parameters() const;

        /**
         * Sets the value of an internal parameter
         * \see UnitParameter::set
         */
        template <typename ID, typename T>
        bool setParam(const ID& a_id, const T& a_value);

        bool hasInput(int a_id) const;

        const string& inputName(int a_id) const;

        const double& readInput(int a_id, int a_offset) const;

        const double* inputSource(int a_id) const;

        const StrMap<InputPort, MAX_INPUTS>& inputs() const;

        bool hasOutput(int a_id) const;

        string outputName(int a_id) const { return m_outputPorts.getNameFromId(a_id); }

        template <typename ID>
        const double& readOutput(const ID& a_id, int a_offset) const;

        const StrMap<OutputPort, MAX_OUTPUTS>& outputs() const;

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

        UnitTypeId getClassIdentifier() const;

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
        virtual Unit* load(const json& j)
        {
            return this;
        }

        /// \returns A unique string that identifies the derived class.
        virtual const string& getClassName() const = 0;

    protected:
        virtual void onParamChange_(int a_paramId) {};

        virtual void onFsChange_() {};

        virtual void onTempoChange_() {};

        virtual void onNoteOn_() {};

        virtual void onNoteOff_() {};

        virtual void onMidiControlChange_(int a_cc, double a_value) {};

        virtual void onPitchBendChange_(double a_value) {};

        virtual void onInputConnection_(int a_inputPort) {};

        virtual void onInputDisconnection_(int a_inputPort) {};

        template <typename ID>
        void writeOutput_(const ID& a_id, int a_offset, const double& a_val);

        virtual void MSFASTCALL process_() GCCFASTCALL = 0;

        int addInput_(const string& a_name, double a_default = 0.0);
        bool addInput_(int a_id, const string& a_name, double a_default = 0.0);

        bool removeInput_(const string& a_name);
        bool removeInput_(int a_id);

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

    protected:
        int m_currentBufferOffset;

    private:
        string m_name;
        StrMap<UnitParameter, MAX_PARAMS> m_parameters;
        StrMap<OutputPort, MAX_OUTPUTS> m_outputPorts;
        StrMap<InputPort, MAX_INPUTS> m_inputPorts;
        Circuit* m_parent;
        AudioConfig m_audioConfig;
        MidiData m_midiData;
    };

    template <typename ID>
    const double& Unit::readOutput(const ID& a_id, int a_offset) const
    {
        return m_outputPorts[a_id][a_offset];
    }

    template <typename ID>
    UnitParameter& Unit::param(const ID& a_id)
    {
        return m_parameters[a_id];
    }

    template <typename ID>
    const UnitParameter& Unit::param(const ID& a_id) const
    {
        return m_parameters[a_id];
    }

    template <typename ID>
    void Unit::writeOutput_(const ID& a_id, int a_offset, const double& a_val)
    {
        m_outputPorts[a_id][a_offset] = a_val;
    }

    template <typename ID, typename T>
    bool Unit::setParam(const ID& a_id, const T& a_value)
    {
        return m_parameters[a_id].set(a_value);
    };
}
#endif
