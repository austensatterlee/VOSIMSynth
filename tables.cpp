#include "tables.h" 
#define LERP(A,B,F) (((B)-(A))*(F)+(A))
double LookupTable::getlinear(const double phase) const{
int int_index = (int)(phase*m_size);
double frac_index = phase*m_size - int_index;
return LERP(m_table[int_index], m_table[int_index + 1], frac_index);
}