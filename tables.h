#ifndef __TABLES__
#define __TABLES__
#define VOSIM_PULSE_COS_SIZE 65536
#define BLEPOS 256
#define BLEPBUFSIZE 32
#define BLEPSIZE 8193
#define SINC_KERNEL_SIZE 51
#define PITCH_TABLE_SIZE 128


class LookupTable {
public:
  LookupTable() :
    m_table(nullptr),
    m_size(0) {}
  LookupTable(const double* table, int size) :
    m_table(table),
    m_size(size) {}
  double getlinear(const double phase) const;
private:
  int m_size;
  const double* m_table;
};


extern const double SINC_KERNEL[SINC_KERNEL_SIZE];
extern const double MINBLEP[BLEPSIZE];
extern const double VOSIM_PULSE_COS[VOSIM_PULSE_COS_SIZE];
extern const double PITCH_TABLE[PITCH_TABLE_SIZE];

const LookupTable lut_sinckernel(SINC_KERNEL, SINC_KERNEL_SIZE);
const LookupTable lut_minblep(MINBLEP, BLEPSIZE);
const LookupTable lut_vosimpulse(VOSIM_PULSE_COS, VOSIM_PULSE_COS_SIZE);
const LookupTable lut_pitchtable(PITCH_TABLE, PITCH_TABLE_SIZE);
#endif