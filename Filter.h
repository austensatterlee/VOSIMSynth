#ifndef __FILTER__
#define __FILTER__
#include "Unit.h"
#include <cstring>
#include <cstdlib>
#define AA_FILTER_SIZE 9
const double AA_FILTER_X[AA_FILTER_SIZE] = { 0.13690162, 1.09521296, 3.83324538, 7.66649075, 9.58311344, 7.66649075, 3.83324538, 1.09521296, 0.13690162 };
const double AA_FILTER_Y[AA_FILTER_SIZE] = { 1., 4.12597592, 8.10936447, 9.53668245, 7.30172852, 3.69021981, 1.21561014, 0.24170958, 0.02785143 };

using namespace std;
namespace syn
{
  template <size_t nX, size_t nY>
  class Filter : public Unit
  {
  protected:
    const double(&XCoefs)[nX];
    const double(&YCoefs)[nY];
    double *YBuf, *XBuf;
    int xBufInd, yBufInd;
    UnitParameter& m_input;
  public:
    Filter(string name, const double(&X)[nX], const double(&Y)[nY]) :
      Unit(name),
      XCoefs(X),
      YCoefs(Y),
      m_input(addParam("input", IParam::kTypeDouble, -1, 1, true))
    {
      XBuf = (double*)malloc(sizeof(double)*nX);
      YBuf = (double*)malloc(sizeof(double)*nY);
      reset();
    };
    Filter(const Filter& filt);
    ~Filter()
    {
      delete[] YBuf;
      delete[] XBuf;
    };
    void reset() { memset(YBuf, 0, nY*sizeof(double)); memset(XBuf, 0, nX*sizeof(double)); xBufInd = 0; yBufInd = 0; };
  protected:
	void process(int bufind) override;
	void onSampleRateChange(double newfs) override;
	void onBufferSizeChange(size_t newbuffersize) override;
	void onTempoChange(double newtempo) override;
  private:
    Unit* cloneImpl() const override { return new Filter(*this); };
	string getClassName() const override;
  };

  template <size_t nX, size_t nY>
  Filter<nX, nY>::Filter(const Filter<nX, nY>& filt) :
    Filter<nX, nY>(filt.m_name, filt.XCoefs, filt.YCoefs)
  {}

	template <size_t nX, size_t nY>
	void Filter<nX, nY>::onSampleRateChange(double newfs)
	{
	}

	template <size_t nX, size_t nY>
	void Filter<nX, nY>::onBufferSizeChange(size_t newbuffersize)
	{
	}

	template <size_t nX, size_t nY>
	void Filter<nX, nY>::onTempoChange(double newtempo)
	{
	}

	template <size_t nX, size_t nY>
	string Filter<nX, nY>::getClassName() const
	{
		return "Filter";
	}

	template <size_t nX, size_t nY>
  void Filter<nX, nY>::process(int bufind)
  {
    XBuf[xBufInd] = readParam(0);
    YBuf[yBufInd] = 0.0;
    double *output = &YBuf[yBufInd];
    int i, j;
    for (i = 0, j = xBufInd; i < nX; i++, j--)
    {
      if (j < 0)
        j = nX - 1;
      YBuf[yBufInd] += XBuf[j] * XCoefs[i];
    }
    for (i = 1, j = yBufInd - 1; i < nY; i++, j--)
    {
      if (j < 0)
        j = nY - 1;
      YBuf[yBufInd] -= YBuf[j] * YCoefs[i] / YCoefs[0];
    }
    xBufInd++;
    if (xBufInd == nX)
      xBufInd = 0;
    yBufInd++;
    if (yBufInd == nY)
      yBufInd = 0;
    m_output[bufind] = *output;
  }
}
#endif