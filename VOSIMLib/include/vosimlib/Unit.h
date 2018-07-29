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
#include "vosimlib/Logging.h"

#include <Eigen/Core>


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

    template<typename T>
    class VOSIMLIB_API Buffer {
    public:
        virtual ~Buffer() = default;
        virtual const T* buf() const = 0;
    };

    template<typename T>
    class VOSIMLIB_API ReadOnlyBuffer : public Buffer<T> {
    public:
        ReadOnlyBuffer() : ReadOnlyBuffer(nullptr) {}
        ReadOnlyBuffer(const T* a_buf) : m_buf(a_buf) {}
        const T* buf() const override { return m_buf; }
    private:
        const T* m_buf;
    };

    template<typename T>
    struct VOSIMLIB_API OutputPort : public Buffer<T> {
        OutputPort()
            : OutputPort(0) {}

        OutputPort(T* a_targetBuf)
            : m_extBuf(a_targetBuf),
              m_intBuf() {}

        explicit OutputPort(int a_bufSize)
            : m_extBuf(nullptr),
              m_intBuf(a_bufSize, 0.0) { }

        T* buf() { return hasExternalBuf() ? m_extBuf : m_intBuf.data(); }
        const T* buf() const override { return hasExternalBuf() ? m_extBuf : m_intBuf.data(); }

        T read(int a_offset) const { return buf()[a_offset]; }

        void setBuf(T* a_targetBuf) { m_extBuf = a_targetBuf == m_intBuf.data() ? nullptr : a_targetBuf; }

        void unsetBuf() { m_extBuf = nullptr; }

        bool hasExternalBuf() const { return m_extBuf != nullptr; }

        void resize(int a_size) { m_intBuf.resize(a_size, 0.0); }

    private:
        T* m_extBuf;
        std::vector<T> m_intBuf;
    };

    template<typename T>
    struct VOSIMLIB_API InputPort {
        InputPort()
            : InputPort(0.0) {}

        explicit InputPort(T a_defVal)
            : defVal(a_defVal),
              src(nullptr) {}
    
    private:
        friend class Unit;

        void connect(const Buffer<T>* a_output) { src = a_output; }

        void disconnect() { src = nullptr; }

        bool isConnected() const { return src != nullptr; }

        T defVal;
        const Buffer<T>* src;
    };

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
     *        void process_() override {
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
        typedef double SampleType;
        typedef Eigen::Array<SampleType, -1, -1, Eigen::RowMajor> dynamic_buffer_t;
        typedef OutputPort<SampleType> OutputPort;
        typedef Buffer<SampleType> Buffer;
        typedef InputPort<SampleType> InputPort;

        Unit();

        explicit Unit(const string& a_name);

        virtual ~Unit() = default;

        const string& name() const { return m_name; }
        void setName(const string& a_name);

        UnitTypeId getClassIdentifier() const;

        /**
         * Processes as many samples as required to fill the internal buffer (see Unit::getBufferSize and Unit::setBufferSize).
         */
        void tick() { process_(); }

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
        void tick(const dynamic_buffer_t& a_inputs, dynamic_buffer_t& a_outputs);

        /**
         * Notify the unit of a sample rate change
         */
        void setFs(double a_newFs);

        /**
         * Notify the unit of a tempo change
         */
        void setTempo(double a_newTempo);

        /**
         * Notify the unit of a "note on" event
         * 
         * \param a_note Midi note number, ranging from 0 to 127, where 60 is middle C
         * \param a_velocity Velocity of the note, ranging from 0 to 127
         */
        void noteOn(int a_note, int a_velocity);

        /**
         * Notify the unit of a "note off" event
         */
        void noteOff(int a_note);

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

        /**
         * Return the unit's sampling frequency
         */
        double fs() const;

        double tempo() const;

        bool isNoteOn() const;

        int note() const;

        int velocity() const;

        /**
         * Determines whether or not this unit should be ticked.
         *
         * Default implementation returns true when a note is on and false
         * otherwise.  Override if the unit should be ticked before note on or
         * after note off events (e.g. an envelope).
         */
        virtual bool isActive() const;

        /**
         * Returns a pointer to the unit's parent Circuit, or nullptr if there is none.
         */
        Circuit* parent() const;

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

        double readInput(int a_id, int a_offset) const;

        const Buffer& inputSource(int a_id) const;

        const StrMap<InputPort, MAX_INPUTS>& inputs() const;

        bool hasOutput(int a_id) const;

        string outputName(int a_id) const;

        double readOutput(int a_id, int a_offset) const;

        const StrMap<OutputPort, MAX_OUTPUTS>& outputs() const;

        OutputPort& output(int a_id);

        int numParams() const;

        int numInputs() const;

        int numOutputs() const;

        /**
        * Connect the specified input port to an output.
        * \param a_inputPort Input port number
        * \param a_output
        */
        void connectInput(int a_inputPort, const Buffer& a_output);
        void connectOutput(int a_outputPort, Unit* a_target, int a_inputPort);

        /**
         * \returns True if the input port points to a non-null location in memory.
         */
        bool isInputConnected(int a_inputPort) const;

        /**
        * Remove the specified connection.
        * \returns True if the connection existed, false if it was already disconnected.
        */
        bool disconnectInput(int a_inputPort);

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

        virtual void onPitchWheelChange_(double a_value) {};

        virtual void onInputConnection_(int a_inputPort) {};

        virtual void onInputDisconnection_(int a_inputPort) {};

        template <typename ID>
        void writeOutput_(const ID& a_id, int a_offset, const double& a_val);

        virtual void process_() = 0;

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
        m_outputPorts[a_id].buf()[a_offset] = a_val;
    }

    template <typename ID, typename T>
    bool Unit::setParam(const ID& a_id, const T& a_value)
    {
        return m_parameters[a_id].set(a_value);
    };
}
#endif
