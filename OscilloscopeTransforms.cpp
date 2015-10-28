#include "Oscilloscope.h"
#include <fftw3.h>
#include <vector>
#include <cmath>

using namespace std;

namespace syn
{
  void Oscilloscope::magnitudeTransform(const std::vector<double>& inputbuf, std::vector<double>& outputbuf, double& minout, double& maxout)
  {
    int N = inputbuf.size();
    int halfN = N / 2 + 1;
    if (outputbuf.size() != halfN)
      outputbuf.resize(halfN, 0.0);
    double* input = (double*)fftw_malloc(sizeof(double) * N);
    fftw_complex* out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * halfN);
    fftw_plan p = fftw_plan_dft_r2c_1d(N, input, out, FFTW_ESTIMATE);

    int i;
    memset(input, 0, sizeof(double)*N);
    for (i = 0; i + inputbuf.size() <= N; i += inputbuf.size())
    {
      memcpy(input + i, inputbuf.data(), sizeof(double)*inputbuf.size());
    }

    fftw_execute(p);

    maxout = 0;
    minout = 0;
    for (i = 0; i < halfN - 1; i++)
    {
      outputbuf[i] = outputbuf[i] + 0.05 * (20 * log10(sqrt(out[i + 1][0] * out[i + 1][0] + out[i + 1][1] * out[i + 1][1])) - outputbuf[i]);
      if (isinf(outputbuf[i]))
      {
        outputbuf[i] = -1;
      }
      if (outputbuf[i] > maxout)
        maxout = outputbuf[i];
      else if (outputbuf[i] < minout)
        minout = outputbuf[i];
    }
    fftw_destroy_plan(p);
    fftw_free(input);
    fftw_free(out);
  }

  void Oscilloscope::inverseTransform(const std::vector<double>& inputbuf, std::vector<double>& outputbuf, double& minout, double& maxout){
    int N = inputbuf.size();
    int twoN = N * 2 - 1;
    if (outputbuf.size() != twoN)
      outputbuf.resize(twoN, 0.0);
    fftw_complex* input = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * N);
    double* out = (double*)fftw_malloc(sizeof(double) * twoN);
    fftw_plan p = fftw_plan_dft_c2r_1d(twoN, input, out, FFTW_ESTIMATE);

    int i;
    memset(input, 0, sizeof(double)*N);
    for (i = 0; i + inputbuf.size() <= N; i += inputbuf.size())
    {
      memcpy(input + i, inputbuf.data(), sizeof(double)*inputbuf.size());
    }

    fftw_execute(p);

    maxout = 0;
    minout = 0;
    for (i = 0; i < twoN; i++)
    {
      outputbuf[i] = out[i];
      if (isinf(outputbuf[i]))
      {
        outputbuf[i] = -1;
      }
      if (outputbuf[i] > maxout)
        maxout = outputbuf[i];
      else if (outputbuf[i] < minout)
        minout = outputbuf[i];
    }
    fftw_destroy_plan(p);
    fftw_free(input);
    fftw_free(out);
  }

  void Oscilloscope::passthruTransform(const std::vector<double>& inputbuf, std::vector<double>& outputbuf, double& minout, double& maxout)
  {
    int N = inputbuf.size();
    if (outputbuf.size() != N)
      outputbuf.resize(N);
    
    maxout = 0;
    minout = 0;
    for (int i = 0; i < N; i++)
    {
      outputbuf[i] = inputbuf[i];
      if(outputbuf[i]>maxout)
        maxout = outputbuf[i];
      else if(outputbuf[i]<minout)
        minout = outputbuf[i];
    }
  }
}