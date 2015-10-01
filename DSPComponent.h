#ifndef __DSPCOMPONENT__
#define __DSPCOMPONENT__

#include "GallantSignal.h"

#define LERP(A,B,F) (((B)-(A))*(F)+(A))

enum MOD_STATE {
  FROZEN = 0,
  ACTIVE
};
template <typename T>
class Modifiable {
protected:
  T m_base;
  T m_offset = 0;
  T m_bias = 0;
  T m_scale = 0;
  MOD_STATE m_state;
public:
  T m_curr = 0;
  Modifiable(MOD_STATE state = ACTIVE) :m_base(0), m_state(state) {};
  Modifiable(T base, MOD_STATE state = ACTIVE) :m_base(base), m_scale(1), m_state(state) {};
  void set(T val) {
    m_base = val;
  };
  void mod(T val) {
    m_offset += val;
  };
  void scale(T val) {
    m_scale *= val;
  }
  void bias(T val) {
    m_bias += val;
  }
  void setState(MOD_STATE state) {
    m_state = state;
  }
  const T& operator()() {
    m_curr = m_scale*(m_base + m_offset) + m_bias;
    if (m_state == ACTIVE) {
      m_offset = 0.0;
      m_scale = 1.0;
      m_bias = 0.0;
    }
    return m_curr;
  };
};

template <class T = double>
class DSPComponent {
public:
  Modifiable<T> m_pGain = Modifiable<T>(1,ACTIVE);
  Modifiable<T> m_pBias = Modifiable<T>(0,ACTIVE);
  virtual ~DSPComponent() {};
  virtual void updateParams() {};
  virtual T process(const T input) = 0;
  virtual void setFs(const double fs) { mFs = fs; };

  template <class U, class V>
  void connectOutputTo(U* obj1, void (V::*func)(T p1)) {
    m_outputs.Connect(obj1, func);
  }
  virtual T getLastOutput() { return m_lastOutput; };
protected:
  virtual void tick() {};
  virtual const T& finishProcessing(const T& output);
  Gallant::Signal1<T> m_outputs;
  double mFs;
  T m_lastOutput = 0;
};

template <class T>
const T& DSPComponent<T>::finishProcessing(const T& output) {
  m_lastOutput = m_pGain()*output + m_pBias();
  m_outputs.Emit(m_lastOutput);
  return m_lastOutput;
}

inline double pitchToFreq(double pitch) { return 440.0 * std::pow(2.0, (pitch - 69.0) / 12.0); }
inline double dbToAmp(double db) { return std::pow(10, 0.05*db); }
inline double ampToDb(double amp) { return 20 * std::log10(amp); }
#endif