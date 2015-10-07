#ifndef __TABLES__
#define __TABLES__
#include "DSPComponent.h"
#define VOSIM_PULSE_COS_SIZE 65536
#define BLEPOS 256
#define BLEPBUFSIZE 32
#define BLEPSIZE 8193
#define SINC_KERNEL_SIZE 51
#define AA_FILTER_SIZE 6


template <class T = double>
class LookupTable {
public:
  LookupTable() :
    m_table(NULL),
    m_size(0) {}
  LookupTable(const T* table, int size) :
    m_table(table),
    m_size(size) {}
  T getlinear(const double phase) const {
    int int_index = (int)(phase*m_size);
    double frac_index = phase*m_size - int_index;
    return LERP(m_table[int_index], m_table[int_index + 1], frac_index);
  }
private:
  int m_size;
  const T* m_table;
};

extern const double SINC_KERNEL[SINC_KERNEL_SIZE];
extern const double MINBLEP[BLEPSIZE];
extern const double VOSIM_PULSE_COS[VOSIM_PULSE_COS_SIZE];
extern const double AA_FILTER_X[AA_FILTER_SIZE + 1];
extern const double AA_FILTER_Y[AA_FILTER_SIZE];

const LookupTable<double> lut_sinckernel(SINC_KERNEL,SINC_KERNEL_SIZE);
const LookupTable<double> lut_minblep(MINBLEP, BLEPSIZE);
const LookupTable<double> lut_vosimpulse(VOSIM_PULSE_COS, VOSIM_PULSE_COS_SIZE);
#endif