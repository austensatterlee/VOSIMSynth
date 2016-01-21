#ifndef __UNIT__
#define __UNIT__
#include "UnitParameter.h"
#include "GallantSignal.h"
#include <unordered_map>
#include <vector>
#include <functional>

using Gallant::Signal1;
using std::unordered_map;
using std::vector;
using std::hash;

namespace syn
{
	typedef unordered_map<string, int> IDMap; //!< string to array index translation map

	class Circuit; // forward decl.
	/**
	 * \class Unit
	 *
	 * \brief Units encapsulate a discrete processor with an internal state
	 *
	 * A unit is composed of N state variables, a single output, and a transition function which produces a new output
	 * given the state of the Unit and the last output. Units expose some of their state variables as inputs in the
	 * form of numbered parameters (UnitParameter).
	 *
	 */
	class Unit
	{
		friend class Circuit;
	public:
		Unit(string name);
		virtual ~Unit();
		void setSampleRate(double newfs);
		void setBufferSize(size_t newbufsize);
		void setTempo(double newtempo);

		unsigned int getClassIdentifier() const
		{
			hash<string> hash_fn;
			return hash_fn(getClassName());
		};

		/*!
		 * \brief Runs the unit for the specified number of ticks (specified with resizeOutputBuffer). The result is accessed via getLastOutputBuffer().
		 */
		void tick();

		double getFs() const
		{
			return m_Fs;
		};

		const vector<double>& getLastOutputBuffer() const
		{
			return m_output;
		};

		double getLastOutput() const
		{
			return m_output[m_output.size() - 1];
		};

		/*!
		 *\brief Modifies the value of the parameter associated with portid.
		 */
		void modifyParameter(int portid, double val, MOD_ACTION action);

		bool hasParameter(string name)
		{
			return m_parammap.find(name) != m_parammap.end();
		};

		double readParam(string pname) const
		{
			return *m_params[m_parammap.at(pname)];
		};

		double readParam(int id) const
		{
			return *m_params[id];
		};

		UnitParameter& getParam(string pname)
		{
			return *m_params[m_parammap.at(pname)];
		}

		UnitParameter& getParam(int pid)
		{
			return *m_params[pid];
		}

		vector<string> getParameterNames() const;
		int getParamId(string name);

		Circuit& getParent() const
		{
			return *m_parent;
		};

		string getName() const
		{
			return m_name;
		}

		void setName(string name)
		{
			m_name = name;
		}

		Unit* clone() const;
	protected:
		typedef vector<UnitParameter*> ParamVec;
		ParamVec m_params;
		IDMap m_parammap;
		string m_name;
		Circuit* m_parent;
		double m_Fs, m_tempo;
		vector<double> m_output;
		virtual void process(int bufind) = 0; // should add its result to m_output[bufind]
		UnitParameter& addParam(string name, int id, IParam::EParamType ptype, double min, double max, double defaultValue, double step, double shape = 1.0, bool isHidden = false, bool canModulate = true);
		UnitParameter& addDoubleParam(string name, double min, double max, double defaultValue, double step, double shape = 1.0, bool isHidden = false, bool canModulate = true);
		UnitParameter& addIntParam(string name, int min, int max, int defaultValue, double shape = 1.0, bool isHidden = false, bool canModulate = true);
		UnitParameter& addBoolParam(string name, bool defaultValue);
		UnitParameter& addEnumParam(string name, const vector<string> choice_names, int defaultValue);
		virtual void onSampleRateChange(double newfs) = 0;
		virtual void onBufferSizeChange(size_t newbuffersize) {};
		virtual void onTempoChange(double newtempo) {};
	private:
		int m_bufind;
		virtual Unit* cloneImpl() const = 0;
		virtual inline string getClassName() const = 0;

		/// Allows parent classes to apply common processing before child class outputs.
		virtual void beginProcessing()
		{
		};

		/// Allows parent classes to apply common processing after child class outputs.
		virtual void finishProcessing()
		{
		}; 
	};

	/*
	 * MISC UTILITY UNITS
	 */
	class AccumulatingUnit : public Unit
	{
	public:
		AccumulatingUnit(string name) : Unit(name),
		                                m_input(addDoubleParam("input", -1, 1, 0.0, 0.0, 1.0, true, true)),
		                                m_gain(addDoubleParam("gain", 0, 1, 0.5, 1e-3, 2.0, false, true ))
		{
		}

		AccumulatingUnit(const AccumulatingUnit& other) : AccumulatingUnit(other.m_name)
		{
		}

		virtual ~AccumulatingUnit()
		{
		};

	protected:
		void onSampleRateChange(double newfs) override {};

		void process(int bufind) override
		{
			m_output[bufind] = m_input * m_gain;
		}

	private:
		UnitParameter& m_input;
		UnitParameter& m_gain;

		Unit* cloneImpl() const override
		{
			return new AccumulatingUnit(*this);
		}

		string getClassName() const override
		{
			return "AccumulatingUnit";
		}
	};
}
#endif

