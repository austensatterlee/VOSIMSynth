#include "Oscilloscope.h"
#include "fftw3.h"
#include <vector>

using namespace std;

namespace syn
{
  void magnitudeTransform(OscilloscopeConfig& oscconfig, IPlugBase* pPlug, vector<double>& inputbuf)
  {
    int N = inputbuf.size();
    int halfN = N / 2;

    // resize outputs based on input size
    if (oscconfig.outputbuf.size() != halfN) {
      oscconfig.outputbuf.resize(halfN, 0.0);
      oscconfig.xaxisticks.resize(halfN, 0.0);
      oscconfig.xaxislbls.resize(halfN);
    }

    // apply window 
    double a[4] = {0.3587, 0.48829, 0.14128, 0.01168}; // blackman-harris
    double winlen = N;
    for(int k=0; k<N; k++)
    {
      double windowpt = a[0] - a[1]*cos(2*PI*k/(N-1)) + a[2]*cos(4*PI*k/(N-1)) - a[3]*cos(6*PI*k/(N-1));
      inputbuf[k] *= windowpt;
    }

    // apply fft
    double* process = static_cast<double*>(fftw_malloc(sizeof(double) * N));
    fftw_complex* out = static_cast<fftw_complex*>(fftw_malloc(sizeof(fftw_complex) * (halfN + 1)));
    fftw_plan p = fftw_plan_dft_r2c_1d(N, process, out, FFTW_ESTIMATE);

    memcpy(process, inputbuf.data(), sizeof(double)*N);

    fftw_execute(p);

    oscconfig.argmax = -1;
    oscconfig.argmin = -1;
    char lblbuf[64];

    // transform output to log scale and
    // update tick positions and labels 
    for (int k = 1; k < halfN + 1; k++)
    {
      int i = k - 1;
      oscconfig.outputbuf[i] = oscconfig.outputbuf[i] + 0.5*(20 * log10(sqrt(out[k][0] * out[k][0] + out[k][1] * out[k][1])) - oscconfig.outputbuf[i]);
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

  void passthruTransform(OscilloscopeConfig& oscconfig, IPlugBase* pPlug, vector<double>& inputbuf)
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