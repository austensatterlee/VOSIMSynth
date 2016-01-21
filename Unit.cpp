#include "Unit.h"
#include <array>

using namespace std;

namespace syn
{
	void Unit::modifyParameter(int portid, double val, MOD_ACTION action)
	{
		m_params[portid]->mod(val, action);
	}

	vector<string> Unit::getParameterNames() const
	{
		vector<string> pnames;
		for (int i = 0; i < m_params.size(); i++)
		{
			pnames.push_back(m_params[i]->getName());
		}
		return pnames;
	}

	Unit* Unit::clone() const
	{
		Unit* u = cloneImpl();
		u->m_name = m_name;
		u->m_Fs = m_Fs;
		u->m_output = m_output;
		u->m_parammap = m_parammap;
		u->m_bufind = m_bufind;
		for (int i = 0; i < m_params.size(); i++)
		{
			u->m_params[i]->mod(*m_params[i], SET);
		}
		return u;
	}

	UnitParameter& Unit::addParam(string name, int id, IParam::EParamType ptype, double min, double max, double defaultValue, double step, double shape, bool isHidden, bool canModulate)
	{
		m_parammap[name] = id;
		if (m_params.size() <= id)
			m_params.resize(id + 1);
		m_params[id] = new UnitParameter(this, name, id, ptype, min, max, defaultValue, step, shape, isHidden, canModulate);
		return *m_params[id];
	}

	UnitParameter& Unit::addDoubleParam(string name, double min, double max, double defaultValue, double step, double shape, bool isHidden, bool canModulate)
	{
		return addParam(name, m_params.size(), IParam::kTypeDouble, min, max, defaultValue, step, shape , isHidden, canModulate);
	}

	UnitParameter& Unit::addIntParam(string name, int min, int max, int defaultValue, double shape, bool isHidden, bool canModulate)
	{
		return addParam(name, m_params.size(), IParam::kTypeInt, min, max, defaultValue, 1.0, shape, isHidden, canModulate);
	}

	UnitParameter& Unit::addBoolParam(string name, bool defaultValue)
	{
		return addParam(name, m_params.size(), IParam::kTypeInt, 0, 1, defaultValue, 1.0, 1.0, false, false);
	}

	UnitParameter& Unit::addEnumParam(string name, const vector<string> choice_names, int defaultValue)
	{
		UnitParameter& param = addParam(name, m_params.size(), IParam::kTypeEnum, 0, choice_names.size(), defaultValue, 1.0, 1.0, false, false);
		for (int i = 0; i < choice_names.size(); i++)
		{
			param.setDisplayText(i, choice_names[i].c_str());
		}
		return param;
	}

	Unit::Unit(string name) :
		m_name(name), 
		m_parent(nullptr),
		m_Fs(44100.0),
		m_tempo(120),
		m_output(1, 0.0),
		m_bufind(0)
	{
	}

	Unit::~Unit()
	{
		// delete allocated parameters
		for (int i = 0; i < m_params.size(); i++)
		{
			delete m_params[i];
		}
	}

	void Unit::setSampleRate(double newfs)
	{
		m_Fs = newfs;
		onSampleRateChange(newfs);
	}

	void Unit::setBufferSize(size_t newbufsize)
	{
		m_output.resize(newbufsize);
		onBufferSizeChange(newbufsize);
	}

	void Unit::setTempo(double newtempo)
	{
		m_tempo = newtempo;
		onTempoChange(newtempo);
	}

	void Unit::tick()
	{
		beginProcessing();
		for (int i = 0; i < m_output.size(); i++)
		{
			for (int j = 0; j < m_params.size(); j++)
			{
				m_params[j]->reset();
				if (m_params[j]->getNumConnections() > 0)
				{
					m_params[j]->pull(i);
				}
			}
			process(i);
			m_bufind = i;
		}
		finishProcessing();
	}

	int Unit::getParamId(string name)
	{
		int paramid;
		try
		{
			paramid = m_parammap.at(name);
		}
		catch (out_of_range exc)
		{
			paramid = -1;
		}
		return paramid;
	}
}

