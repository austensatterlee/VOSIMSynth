#include "Oscilloscope.h"
#include "fftw3.h"
#include <vector>

using namespace std;

namespace syn
{
  void magnitudeTransform(OscilloscopeConfig& oscconfig, IPlugBase* pPlug, const vector<double>& inputbuf)
  {
    int N = inputbuf.size();
    int halfN = N / 2;
    if (oscconfig.outputbuf.size() != halfN) {
      oscconfig.outputbuf.resize(halfN, 0.0);
      oscconfig.xaxisticks.resize(halfN, 0.0);
      oscconfig.xaxislbls.resize(halfN);
    }
    double* process = static_cast<double*>(fftw_malloc(sizeof(double) * N));
    fftw_complex* out = static_cast<fftw_complex*>(fftw_malloc(sizeof(fftw_complex) * (halfN + 1)));
    fftw_plan p = fftw_plan_dft_r2c_1d(N, process, out, FFTW_ESTIMATE);

    memcpy(process, inputbuf.data(), sizeof(double)*N);

    fftw_execute(p);

    oscconfig.argmax = -1;
    oscconfig.argmin = -1;
    char lblbuf[64];
    for (int k = 1; k < halfN + 1; k++)
    {
      int i = k - 1;
      oscconfig.outputbuf[i] = oscconfig.outputbuf[i] + 0.7*(20 * log10(sqrt(out[k][0] * out[k][0] + out[k][1] * out[k][1])) - oscconfig.outputbuf[i]);
      oscconfig.xaxisticks[i] = log10((k / double(halfN))*pPlug->GetSampleRate());
      snprintf(lblbuf, 64, "%g", (k / double(N))*pPlug->GetSampleRate());
      oscconfig.xaxislbls[i] = string(lblbuf);
      if (isinf(oscconfig.outputbuf[i]))
      {
        oscconfig.outputbuf[i] = -1;
      }
      if (oscconfig.argmax == -1 || oscconfig.outputbuf[i] > oscconfig.outputbuf[oscconfig.argmax])
        oscconfig.argmax = i;
      else if (oscconfig.argmin == -1 || oscconfig.outputbuf[i] < oscconfig.outputbuf[oscconfig.argmin])
        oscconfig.argmin = i;
    }
    fftw_destroy_plan(p);
    fftw_free(process);
    fftw_free(out);
  }

  void passthruTransform(OscilloscopeConfig& oscconfig, IPlugBase* pPlug, const vector<double>& inputbuf)
  {
    int N = inputbuf.size();
    if (oscconfig.outputbuf.size() != N) {
      oscconfig.outputbuf.resize(N, 0.0);
      oscconfig.xaxisticks.resize(N, 0.0);
      oscconfig.xaxislbls.resize(N);
    }

    oscconfig.argmax = -1;
    oscconfig.argmin = -1;
    char lblbuf[64];
    for (int i = 0; i < N; i++)
    {
      oscconfig.outputbuf[i] = inputbuf[i];
      oscconfig.xaxisticks[i] = i / double(N)*pPlug->GetSampleRate();
      snprintf(lblbuf, 64, "%f", i / double(pPlug->GetSampleRate()));
      oscconfig.xaxislbls[i] = string(lblbuf);
      if (oscconfig.argmax == -1 || oscconfig.outputbuf[i] > oscconfig.outputbuf[oscconfig.argmax])
        oscconfig.argmax = i;
      else if (oscconfig.argmin == -1 || oscconfig.outputbuf[i] < oscconfig.outputbuf[oscconfig.argmin])
        oscconfig.argmin = i;
    }
  }
}