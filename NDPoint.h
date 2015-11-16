#ifndef __NDPoint__
#define __NDPoint__

#include <stdarg.h>
#include <algorithm>

namespace syn
{
  template <int ND = 2, typename T=double>
  class NDPoint
  {
  protected:
    T m_pvec[ND];
  public:
    NDPoint()
    {
      memset(m_pvec, 0, ND*sizeof(T));
    }
    NDPoint(const T a_tuple[ND])
    {
      std::copy(a_tuple, a_tuple + ND, m_pvec);
    }
    NDPoint(const NDPoint<ND,T>& a_pt)
    {
      std::copy(a_pt.m_pvec, a_pt.m_pvec + ND, m_pvec);
    }
    NDPoint(T a_n1, ...)
    {
      va_list vl;
      va_start(vl, a_n1);
      T value = a_n1;
      for (int i = 0; i < ND; i++)
      {
        m_pvec[i] = value;
        value = va_arg(vl, T);
      }
      va_end(vl);
    }
    NDPoint<ND,T> operator*(T a_num) const
    {
      T newpos[ND];
      for (int i = 0; i < ND; i++)
      {
        newpos[i] = m_pvec[i] * a_num;
      }
      return NDPoint(newpos);
    }
    NDPoint<ND,T> operator*(const NDPoint<ND>& a_pt) const
    {
      double newpos[ND];
      for (int i = 0; i < ND; i++)
      {
        newpos[i] = m_pvec[i] * a_pt.m_pvec[i];
      }
      return NDPoint(newpos);
    }
    NDPoint<ND,T> operator+(T a_num) const
    {
      T newpos[ND];
      for (int i = 0; i < ND; i++)
      {
        newpos[i] = m_pvec[i] + a_num;
      }
      return NDPoint(newpos);
    }
    NDPoint<ND,T> operator+(const NDPoint<ND,T>& a_pt) const
    {
      T newpos[ND];
      for (int i = 0; i < ND; i++)
      {
        newpos[i] = m_pvec[i] + a_pt.m_pvec[i];
      }
      return NDPoint(newpos);
    }
    NDPoint<ND,T> operator-(T a_num) const
    {
      T newpos[ND];
      for (int i = 0; i < ND; i++)
      {
        newpos[i] = m_pvec[i] - a_num;
      }
      return NDPoint(newpos);
    }
    NDPoint<ND,T> operator-(const NDPoint<ND,T>& a_pt) const
    {
      T newpos[ND];
      for (int i = 0; i < ND; i++)
      {
        newpos[i] = m_pvec[i] - a_pt.m_pvec[i];
      }
      return NDPoint(newpos);
    }
    NDPoint<ND,T> operator/(const NDPoint<ND,T>& a_pt) const
    {
      T newpos[ND];
      for (int i = 0; i < ND; i++)
      {
        newpos[i] = m_pvec[i] / a_pt.m_pvec[i];
      }
      return NDPoint(newpos);
    }
    NDPoint<ND,T> operator/(double a_num) const
    {
      T newpos[ND];
      for (int i = 0; i < ND; i++)
      {
        newpos[i] = m_pvec[i] / a_num;
      }
      return NDPoint(newpos);
    }
    void operator*=(T a_num)
    {
      for (int i = 0; i < ND; i++)
      {
        m_pvec[i] *= a_num;
      }
    }
    void operator*=(const NDPoint<ND,T>& a_pt)
    {
      for (int i = 0; i < ND; i++)
      {
        m_pvec[i] *= a_pt.m_pvec[i];
      }
    }
    void operator+=(T a_num)
    {
      for (int i = 0; i < ND; i++)
      {
        m_pvec[i] += a_num;
      }
    }
    void operator+=(const NDPoint<ND,T>& a_pt)
    {
      for (int i = 0; i < ND; i++)
      {
        m_pvec[i] += a_pt.m_pvec[i];
      }
    }
    void operator/=(const NDPoint<ND,T>& a_pt)
    {
      for (int i = 0; i < ND; i++)
      {
        m_pvec[i] /= a_pt.m_pvec[i];
      }
    }
    void operator/=(T a_num)
    {
      for (int i = 0; i < ND; i++)
      {
        m_pvec[i] /= a_num;
      }
    }
    void operator-=(T a_num)
    {
      for (int i = 0; i < ND; i++)
      {
        m_pvec[i] -= a_num;
      }
    }
    void operator-=(const NDPoint<ND,T>& a_pt)
    {
      for (int i = 0; i < ND; i++)
      {
        m_pvec[i] -= a_pt.m_pvec[i];
      }
    }
    bool operator==(T a_num) const
    {
      for (int i = 0; i < ND; i++)
      {
        if (m_pvec[i] != a_num)
          return false;
      }
      return true;
    }
    bool operator==(const NDPoint<ND,T>& a_pt)
    {
      for (int i = 0; i < ND; i++)
      {
        if (m_pvec[i] != a_pt.m_pvec[i])
          return false;
      }
      return true;
    }
    bool operator<(const NDPoint<ND,T>& a_pt)
    {
      for (int i = 0; i < ND; i++)
      {
        if (m_pvec[i] >= a_pt.m_pvec[i])
          return false;
      }
      return true;
    }
    T& operator[](const int& index)
    {
      return m_pvec[index];
    }  
    const T& operator[](const int& index) const
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
    double distFrom(const NDPoint<ND,T>& a_pt) const
    {
      NDPoint<ND,T> pt = *this - a_pt;
      return pt.mag();
    }
    void clamp(const NDPoint<ND,T>& a_minpt, const NDPoint<ND,T>& a_maxpt)
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

  template <int ND, typename T=double>
  NDPoint<ND,T> getZeros()
  {
    T zeros[ND];
    std::fill_n(zeros, ND, 0.0);
    return NDPoint<ND>(zeros);
  }
  template <int ND, typename T = double>
  NDPoint<ND,T> getOnes()
  {
    T ones[ND];
    std::fill_n(ones, ND, 1.0);
    return NDPoint<ND,T>(ones);
  }
  template <int ND, typename T = double>
  NDPoint<ND,T> getUnitv(int a_dir)
  {
    _ASSERT(a_dir >= 0 && a_dir < ND);
    T pvec[ND];
    std::fill_n(pvec, ND, 0.0);
    pvec[a_dir] = 1.0;
    return NDPoint<ND,T>(pvec);
  }
}
#endif // __NDPoint__