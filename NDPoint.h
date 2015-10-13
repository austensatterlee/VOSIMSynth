#ifndef __NDPoint__
#define __NDPoint__

#include <stdarg.h>
#include <algorithm>

namespace syn
{
  template <int ND = 2>
  class NDPoint
  {
  private:
    double m_pvec[ND];
  public:
    NDPoint()
    {
      memset(m_pvec, 0, ND*sizeof(double));
    }
    NDPoint(const double a_tuple[ND])
    {
      std::copy(a_tuple, a_tuple + ND, m_pvec);
    }
    NDPoint(const NDPoint<ND>& a_pt)
    {
      std::copy(a_pt.m_pvec, a_pt.m_pvec + ND, m_pvec);
    }
    NDPoint(double a_n1, ...)
    {
      va_list vl;
      va_start(vl, a_n1);
      double value = a_n1;
      for (int i = 0; i < ND; i++)
      {
        m_pvec[i] = value;
        value = va_arg(vl, double);
      }
      va_end(vl);
    }
    NDPoint<ND> operator*(double a_num) const
    {
      double newpos[ND];
      for (int i = 0; i < ND; i++)
      {
        newpos[i] = m_pvec[i] * a_num;
      }
      return NDPoint(newpos);
    }
    NDPoint<ND> operator*(const NDPoint<ND>& a_pt) const
    {
      double newpos[ND];
      for (int i = 0; i < ND; i++)
      {
        newpos[i] = m_pvec[i] * a_pt.m_pvec[i];
      }
      return NDPoint(newpos);
    }
    NDPoint<ND> operator+(double a_num) const
    {
      double newpos[ND];
      for (int i = 0; i < ND; i++)
      {
        newpos[i] = m_pvec[i] + a_num;
      }
      return NDPoint(newpos);
    }
    NDPoint<ND> operator+(const NDPoint<ND>& a_pt) const
    {
      double newpos[ND];
      for (int i = 0; i < ND; i++)
      {
        newpos[i] = m_pvec[i] + a_pt.m_pvec[i];
      }
      return NDPoint(newpos);
    }
    NDPoint<ND> operator-(double a_num) const
    {
      int newpos[ND];
      for (int i = 0; i < ND; i++)
      {
        newpos[i] = m_pvec[i] - a_num;
      }
      return NDPoint(newpos);
    }
    NDPoint<ND> operator-(const NDPoint<ND>& a_pt) const
    {
      double newpos[ND];
      for (int i = 0; i < ND; i++)
      {
        newpos[i] = m_pvec[i] - a_pt.m_pvec[i];
      }
      return NDPoint(newpos);
    }
    NDPoint<ND> operator/(const NDPoint<ND>& a_pt) const
    {
      double newpos[ND];
      for (int i = 0; i < ND; i++)
      {
        newpos[i] = m_pvec[i] / a_pt.m_pvec[i];
      }
      return NDPoint(newpos);
    }
    NDPoint<ND> operator/(double a_num) const
    {
      double newpos[ND];
      for (int i = 0; i < ND; i++)
      {
        newpos[i] = m_pvec[i] / a_num;
      }
      return NDPoint(newpos);
    }
    void operator*=(double a_num)
    {
      for (int i = 0; i < ND; i++)
      {
        m_pvec[i] *= a_num;
      }
    }
    void operator*=(const NDPoint& a_pt)
    {
      for (int i = 0; i < ND; i++)
      {
        m_pvec[i] *= a_pt.m_pvec[i];
      }
    }
    void operator+=(double a_num)
    {
      for (int i = 0; i < ND; i++)
      {
        m_pvec[i] += a_num;
      }
    }
    void operator+=(const NDPoint& a_pt)
    {
      for (int i = 0; i < ND; i++)
      {
        m_pvec[i] += a_pt.m_pvec[i];
      }
    }
    void operator/=(const NDPoint<ND>& a_pt)
    {
      for (int i = 0; i < ND; i++)
      {
        m_pvec[i] /= a_pt.m_pvec[i];
      }
    }
    void operator/=(double a_num)
    {
      for (int i = 0; i < ND; i++)
      {
        m_pvec[i] /= a_num;
      }
    }
    void operator-=(double a_num)
    {
      for (int i = 0; i < ND; i++)
      {
        m_pvec[i] -= a_num;
      }
    }
    void operator-=(const NDPoint& a_pt)
    {
      for (int i = 0; i < ND; i++)
      {
        m_pvec[i] -= a_pt.m_pvec[i];
      }
    }
    bool operator==(double a_num) const
    {
      for (int i = 0; i < ND; i++)
      {
        if (m_pvec[i] != a_num)
          return false;
      }
      return true;
    }
    bool operator==(const NDPoint<ND>& a_pt)
    {
      for (int i = 0; i < ND; i++)
      {
        if (m_pvec[i] != a_pt.m_pvec[i])
          return false;
      }
      return true;
    }
    bool operator<(const NDPoint<ND>& a_pt)
    {
      for (int i = 0; i < ND; i++)
      {
        if (m_pvec[i] >= a_pt.m_pvec[i])
          return false;
      }
      return true;
    }
    const double& operator[](const int& index) const
    {
      return m_pvec[index];
    }
    double mag() const
    {
      double dmag = 0;
      for (int i = 0; i < ND; i++)
      {
        dmag += m_pvec[i] * m_pvec[i];
      }
      dmag = sqrt(dmag);
      return dmag;
    }
    double distFrom(const NDPoint<ND>& a_pt) const
    {
      NDPoint<ND> pt = *this - a_pt;
      return pt.mag();
    }
    void clamp(const NDPoint<ND>& a_minpt, const NDPoint<ND>& a_maxpt)
    {
      for (int i = 0; i < ND; i++)
      {
        if (m_pvec[i] > a_maxpt[i])
        {
          m_pvec[i] = a_maxpt[i];
        }
        else if (m_pvec[i] < a_minpt[i])
        {
          m_pvec[i] = a_minpt[i];
        }
      }
    }
  };

  template <int ND>
  NDPoint<ND> getZeros()
  {
    double zeros[ND];
    std::fill_n(zeros, ND, 0.0);
    return NDPoint<ND>(zeros);
  }
  template <int ND>
  NDPoint<ND> getOnes()
  {
    double ones[ND];
    std::fill_n(ones, ND, 1.0);
    return NDPoint<ND>(ones);
  }
  template <int ND>
  NDPoint<ND> getUnitv(int a_dir)
  {
    _ASSERT(a_dir >= 0 && a_dir < ND);
    double pvec[ND];
    std::fill_n(pvec, ND, 0.0);
    pvec[a_dir] = 1.0;
    return NDPoint<ND>(pvec);
  }
}
#endif // __NDPoint__