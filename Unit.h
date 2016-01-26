#ifndef __UNIT__
#define __UNIT__
#include "UnitParameter.h"
#include "GallantSignal.h"
#include <unordered_map>
#include <vector>

using Gallant::Signal1;
using std::unordered_map;
using std::vector;
using std::hash;

namespace syn
{
	typedef unordered_map<string, int> IDMap; //!< string to array index translation map

	class Circuit; // forward decl.

	enum UNIT_TYPE
	{
		STD_UNIT,
		SOURCE_UNIT
	};

	struct UnitSample
	{
		double output[2];
		double getCollapsed() const
		{
			return 0.5*(output[0] + output[1]);
		}
		operator double() const {
			return getCollapsed();
		}
		double& operator[](int index)
		{
			return output[index];
		}

		double operator[](int index) const
		{
			return output[index];
		}
	};

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

		Unit::Unit(string name) :
			m_name(name),
			m_parent(nullptr),
			m_Fs(44100.0),
			m_tempo(120),
			m_output(1, { 0.0,0.0 }),
			m_bufind(0) {}

		virtual ~Unit();
		void setSampleRate(double newfs);
		void setBufferSize(size_t newbufsize);
		void setTempo(double newtempo);

		/// Used for dynamic down-casting	 
		virtual UNIT_TYPE getUnitType() const {
			return STD_UNIT;
		}

		unsigned int getClassIdentifier() const {
			hash<string> hash_fn;
			return hash_fn(getClassName());
		};

		/// Runs the unit for the specified number of ticks (specified with resizeOutputBuffer). The result is accessed via getLastOutputBuffer().		 
		void tick();

		double getFs() const {
			return m_Fs;
		};

		const vector<UnitSample>& getLastOutputBuffer() const {
			return m_output;
		};

		UnitSample getLastOutput() const {
			return m_output[m_bufind];
		};

		/*!
		 *\brief Modifies the value of the parameter associated with portid.
		 */
		void modifyParameter(int portid, double val, MOD_ACTION action);

		bool hasParameter(string name) {
			return m_parammap.find(name) != m_parammap.end();
		};

		double readParam(string pname) const {
			return *m_params[m_parammap.at(pname)];
		};

		double readParam(int id) const {
			return *m_params[id];
		};

		UnitParameter& getParam(string pname) {
			return *m_params[m_parammap.at(pname)];
		}

		UnitParameter& getParam(int pid) {
			return *m_params[pid];
		}

		vector<string> getParameterNames() const;

		/// Finds the integer parameter id associated with the given name. Returns -1 if no parameter is associated with the given name.
		int getParamId(string name);

		Circuit& getParent() const {
			return *m_parent;
		};

		string getName() const {
			return m_name;
		}

		void setName(string name) {
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
		vector<UnitSample> m_output;
		int m_bufind;
		virtual void process(int bufind) = 0; // should add its result to m_output[bufind]
		UnitParameter& addParam(string name, int id, IParam::EParamType ptype, double min, double max, double defaultValue, double step, double shape = 1.0, bool canEdit = true, bool canModulate = true);
		UnitParameter& addDoubleParam(string name, double min = 0, double max = 1, double defaultValue = 0, double step = 1e-2, double shape = 1.0, bool canEdit = true, bool canModulate = true);
		UnitParameter& addIntParam(string name, int min, int max, int defaultValue, double shape = 1.0, bool canEdit = true, bool canModulate = true);
		UnitParameter& addBoolParam(string name, bool defaultValue);
		UnitParameter& addEnumParam(string name, const vector<string> choice_names, int defaultValue);
		virtual void onSampleRateChange(double newfs) = 0;

		virtual void onBufferSizeChange(size_t newbuffersize) {};

		virtual void onTempoChange(double newtempo) {};

	private:
		virtual Unit* cloneImpl() const = 0;
		virtual inline string getClassName() const = 0;

		/// Allows parent classes to apply common processing before child class outputs.
		virtual void beginProcessing() { };

		/// Allows parent classes to apply common processing after child class outputs.
		virtual void finishProcessing() { };
	};
}
#endif

