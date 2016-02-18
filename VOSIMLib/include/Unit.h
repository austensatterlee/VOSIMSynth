#ifndef __UNIT__
#define __UNIT__

#include "UnitParameter.h"
#include "SignalBus.h"
#include "UnitConnectionBus.h"
#include "NamedContainer.h"
#include <vector>
#include <memory>

using std::shared_ptr;
using std::make_shared;

namespace syn {
    class Circuit;

    struct AudioConfig {
        double fs;
        double tempo;
    };

    struct MidiData {
        int note;
        int velocity;
        bool isNoteOn;
    };

    /**
     * \class Unit
     *
     * \brief Units encapsulate a discrete processor with an internal state
     *
     * A unit is composed of internal state variables, a number of outputs and inputs,
     * and a transition function, Unit::process_, which produces a new set of outputs given the state
     * of the Unit and the current input.
     *
     * New units should be derived by subclassing UnitCloneable, with Unit::addInput_, Unit::addOutput_,
     * and Unit::addParameter_ called in the constructor some of number of times according to
     * how many inputs, outputs, and parameters are needed (respectively).
     *
     */
    class Unit {
    public:
        Unit();

        Unit(const string& a_name);

        virtual ~Unit()
        { };

        void tick();

        void reset();

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

        const Circuit* const getParent() const;

        template<typename ID>
        const UnitParameter& getParameter(const ID& a_identifier) const;

        template<typename ID, typename T>
        bool setParameter(const ID& a_identifier, const T& a_value);

        template<typename ID, typename T>
        bool setParameterNorm(const ID& a_identifier, const T& a_value);

		template<typename ID>
		string getInputName(const ID& a_identifier) const;

		template<typename ID>
		string getOutputName(const ID& a_identifier) const;

        template<typename ID>
        const Signal& getOutputChannel(const ID& a_identifier) const;

        template<typename ID>
        const Signal& getInputChannel(const ID& a_identifier) const;

        template<typename ID, typename T>
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
        virtual void onParamChange_(int a_paramId)
        { };

        virtual void onFsChange_()
        { };

        virtual void onTempoChange_()
        { };

        virtual void onNoteOn_()
        { };

        virtual void onNoteOff_()
        { };

		template<typename ID>
		UnitParameter& getParameter_(const ID& a_identifier);

        virtual void process_(const SignalBus& a_inputs, SignalBus& a_outputs) = 0;

        int addInput_(const string& a_name);

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


    template<typename ID>
    const Signal& Unit::getOutputChannel(const ID& a_identifier) const
    {
        return m_outputSignals.getChannel(a_identifier);
    }

    template<typename ID>
    const Signal& Unit::getInputChannel(const ID& a_identifier) const
    {
        return m_inputSignals.getChannel(a_identifier);
    }

    template<typename ID, typename T>
    bool Unit::setInputChannel(const ID& a_identifier, const T& a_val)
    {
        return m_inputSignals.setChannel(a_identifier, a_val);
    }

	template <typename ID>
	UnitParameter& Unit::getParameter_(const ID& a_identifier) {
		return m_parameters[a_identifier];
    };

    template<typename ID>
    const UnitParameter& Unit::getParameter(const ID& a_identifier) const
    {
        return m_parameters[a_identifier];
    }

    template<typename ID, typename T>
    bool Unit::setParameter(const ID& a_identifier, const T& a_value)
    {
        if (!m_parameters.find(a_identifier))
            return false;
        if (m_parameters[a_identifier].set(a_value)) {
            int id = m_parameters.getItemIndex(a_identifier);
            onParamChange_(id);
            return true;
        } else {
            return false;
        }
    };

    template<typename ID, typename T>
    bool Unit::setParameterNorm(const ID& a_identifier, const T& a_value)
    {
        if (!m_parameters.find(a_identifier))
            return false;
        if (m_parameters[a_identifier].setNorm(a_value)) {
            int id = m_parameters.getItemIndex(a_identifier);
            onParamChange_(id);
            return true;
        } else {
            return false;
        }
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

