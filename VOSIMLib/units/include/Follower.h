/**
* \file Follower.h
* \brief
* \details
* \author Austen Satterlee
*/
#include "Unit.h"
#include "DSPMath.h"

using namespace std;

namespace syn {
	/**
	* Full-wave rectifier
	*/
	class FollowerUnit : public Unit {
	public:
		FollowerUnit(const string& a_name);

		FollowerUnit(const FollowerUnit& a_rhs);

	protected:
		void process_(const SignalBus& a_inputs, SignalBus& a_outputs) override;
	private:
		string _getClassName() const override
		{
			return "FollowerUnit";
		}

		Unit* _clone() const override { return new FollowerUnit(*this); }
	private:
		double m_w;
		double m_output;

		int m_pAlpha;
		int m_pBeta;
	};
};