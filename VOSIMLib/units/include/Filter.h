#ifndef __FILTER__
#define __FILTER__
#include "Unit.h"
#include <cstring>
#include <cstdlib>

using namespace std;

namespace syn
{
	template <size_t nX, size_t nY>
	class Filter : public Unit
	{
	protected:
		double XCoefs[nX];
		double YCoefs[nY];
		double XBuf[nX];
		double YBuf[nY];
		int xBufInd, yBufInd;
		int m_pInput;
	public:
		Filter(string a_name, const double& a_X[nX], const double& a_Y[nY]) :
				Unit(a_name),
			m_pInput(addParameter_(UnitParameter("input", -1.0, 1.0, 0.0)))
		{
            addInput_("in");
            addOutput_("out");
			memcpy(XCoefs, a_X, sizeof(double)*nX);
			memcpy(YCoefs, a_Y, sizeof(double)*nY);
			reset_();
		};

		Filter(const Filter& a_rhs) :
				Filter(a_rhs.getName(),XCoefs,YCoefs)
		{

		}

		virtual ~Filter() { };

	protected:
		void process_(const SignalBus& a_inputs, SignalBus& a_outputs) override;
        void reset_() {
            memset(YBuf, 0, nY * sizeof(double));
            memset(XBuf, 0, nX * sizeof(double));
            xBufInd = 0;
            yBufInd = 0;
        };
	private:
		string _getClassName() const override { return "Filter"; };
	};

	template <size_t nX, size_t nY>
	void Filter<nX, nY>::process_(const SignalBus& a_inputs, SignalBus& a_outputs) {
		XBuf[xBufInd] = a_inputs.getValue(0);
		YBuf[yBufInd] = 0.0;
		double* output = &YBuf[yBufInd];
		int i, j;
		for (i = 0 , j = xBufInd; i < nX; i++ , j--) {
			if (j < 0)
				j = nX - 1;
			YBuf[yBufInd] += XBuf[j] * XCoefs[i];
		}
		for (i = 1 , j = yBufInd - 1; i < nY; i++ , j--) {
			if (j < 0)
				j = nY - 1;
			YBuf[yBufInd] -= YBuf[j] * YCoefs[i];
		}
		xBufInd++;
		if (xBufInd == nX)
			xBufInd = 0;
		yBufInd++;
		if (yBufInd == nY)
			yBufInd = 0;
		a_outputs.setChannel(0,*output);
	}
}
#endif

