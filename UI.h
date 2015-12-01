#ifndef __UI__
#define __UI__
#include "IControl.h"
#include "NDPoint.h"
#include <vector>
#define SCREEN_PAD_L 25
#define X_PAD 10
#define Y_PAD 32
#define CELL_SIZE 32

using namespace std;
namespace syn
{
  class ColorPoint : public NDPoint<4>
  {
  protected:
    unsigned int m_word;
  public:
    ColorPoint(unsigned int word) :
      m_word(word),
      NDPoint<4>((double)(word >> 24), (double)((word >> 16) & 0xFF), (double)((word >> 8) & 0xFF), (double)(word & 0xFF))
    {}
    ColorPoint(const NDPoint<4>& pt) :
      NDPoint<4>(pt)
    {
    }
    operator IColor() const
    {
      return IColor((int)m_pvec[0], (int)m_pvec[1], (int)m_pvec[2], (int)m_pvec[3]);
    }
  };

  const vector<ColorPoint> globalPalette = {
    {0xFF06070E},
    {0xFF49524A},
    {0xFF94A187},
    {0xFFC5AFA0},
    {0xFFE9BCB7}
  };
}
#endif