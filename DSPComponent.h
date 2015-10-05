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
  T m_scale = 1.0;
  T m_sidechain = 1.0;
  MOD_STATE m_state;
public:
  T m_curr = 0;
  Modifiable(MOD_STATE state = ACTIVE) :m_base(0), m_state(state) {};
  Modifiable(T base, MOD_STATE state = ACTIVE) :m_base(base), m_state(state) {};
  void set(T val) {
    m_base = val;
  };
  void bias(T val) {
    m_bias = val;
  }
  void mod(T val) {
    m_offset += val;
  };
  void scale(T val) {
    m_scale *= val;
  }
  void sc(T val) {
    m_sidechain *= val;
  }
  const T& operator()(MOD_STATE state) {
    m_state = state;
    return (*this)();
  }
  const T& operator()(){
    m_curr = m_scale*m_base + m_sidechain*(m_offset) + m_bias;
    if (m_state == ACTIVE) {
      m_offset = 0.0;
      m_scale = 1.0;
      m_sidechain = 1.0;
    }
    return m_curr;
  };
  const T& get() {
    return m_curr;
  }
};

template <class T = double>
class DSPComponent {
public:
  Modifiable<T> m_pGain = Modifiable<T>(1,ACTIVE);
  Modifiable<T> m_pBias = Modifiable<T>(0,ACTIVE);
  virtual void updateParams(MOD_STATE state=ACTIVE) {
    m_pGain(state);
    m_pBias(state);
  };
  void freezeParams() {
    updateParams(FROZEN);
  }
  virtual T process(const T input) = 0;
  void setFs(const double fs) { mFs = fs; };

  template <class U, class V>
  void connectOutputTo(U* obj1, void (V::*func)(T p1)) {
    m_outputs.Connect(obj1, func);
  }
  template <class U, class V>
  void disconnectOutputTo(U* obj1, void (V::*func)(T p1)) {
    m_outputs.Disconnect(obj1, func);
  }
  T getLastOutput() { return m_lastOutput; };
  Gallant::Signal0<> triggerOut;
  virtual int getSamplesPerPeriod() const = 0;
protected:
  virtual void tick() {};
  const T& finishProcessing(const T& output);
  Gallant::Signal1<T> m_outputs;
  double mFs;
  T m_lastOutput = 0;
};

template <class T>
const T& DSPComponent<T>::finishProcessing(const T& output) {
  m_lastOutput = m_pGain.get()*output + m_pBias.get();
  m_outputs.Emit(m_lastOutput);
  return m_lastOutput;
}

inline double pitchToFreq(double pitch) { return 440.0 * std::pow(2.0, (pitch - 69.0) / 12.0); }
inline double dbToAmp(double db) { return std::pow(10, 0.05*db); }
inline double ampToDb(double amp) { return 20 * std::log10(amp); }
inline int gcd(int a, int b) {
  int c;
  while (a != 0) {
    c = a; a = b%a;  b = c;
  }
  return b;
}
#endif