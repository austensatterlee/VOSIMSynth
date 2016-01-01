#ifndef __NDPoint__
#define __NDPoint__
/**
 * Defines a general purpose point class templated by number of dimensions.
 */
#include <stdarg.h>

namespace syn
{
  template <int ND = 2, typename T = double>
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
    NDPoint(const NDPoint<ND, T>& a_pt)
    {
      std::copy(a_pt.m_pvec, a_pt.m_pvec + ND, m_pvec);
    }
    /**
     * Variable argument constructor. Accepts one argument for each dimension.
     */
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
    /**
     * Multiply all components by a scalar.
     */
    NDPoint<ND, T> operator*(T a_num) const
    {
      T newpos[ND];
      for (int i = 0; i < ND; i++)
      {
        newpos[i] = m_pvec[i] * a_num;
      }
      return NDPoint(newpos);
    }
    /**
    * Component-wise multiplication.
    */
    NDPoint<ND, T> operator*(const NDPoint<ND>& a_pt) const
    {
      double newpos[ND];
      for (int i = 0; i < ND; i++)
      {
        newpos[i] = m_pvec[i] * a_pt.m_pvec[i];
      }
      return NDPoint(newpos);
    }
    /**
    * Add a scalar amount to each component.
    */
    NDPoint<ND, T> operator+(T a_num) const
    {
      T newpos[ND];
      for (int i = 0; i < ND; i++)
      {
        newpos[i] = m_pvec[i] + a_num;
      }
      return NDPoint(newpos);
    }
    /**
    * Component-wise addition.
    */
    NDPoint<ND, T> operator+(const NDPoint<ND, T>& a_pt) const
    {
      T newpos[ND];
      for (int i = 0; i < ND; i++)
      {
        newpos[i] = m_pvec[i] + a_pt.m_pvec[i];
      }
      return NDPoint(newpos);
    }
    /**
    * Subtract a scalar amount from all components
    */
    NDPoint<ND, T> operator-(T a_num) const
    {
      T newpos[ND];
      for (int i = 0; i < ND; i++)
      {
        newpos[i] = m_pvec[i] - a_num;
      }
      return NDPoint(newpos);
    }
    /**
    * Component-wise subtraction.
    */
    NDPoint<ND, T> operator-(const NDPoint<ND, T>& a_pt) const
    {
      T newpos[ND];
      for (int i = 0; i < ND; i++)
      {
        newpos[i] = m_pvec[i] - a_pt.m_pvec[i];
      }
      return NDPoint(newpos);
    }
    /**
    * Divide each component by a scalar.
    */
    NDPoint<ND, T> operator/(double a_num) const
    {
      T newpos[ND];
      for (int i = 0; i < ND; i++)
      {
        newpos[i] = m_pvec[i] / a_num;
      }
      return NDPoint(newpos);
    }
    /**
    * Component-wise division.
    */
    NDPoint<ND, T> operator/(const NDPoint<ND, T>& a_pt) const
    {
      T newpos[ND];
      for (int i = 0; i < ND; i++)
      {
        newpos[i] = m_pvec[i] / a_pt.m_pvec[i];
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
    void operator*=(const NDPoint<ND, T>& a_pt)
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
    void operator+=(const NDPoint<ND, T>& a_pt)
    {
      for (int i = 0; i < ND; i++)
      {
        m_pvec[i] += a_pt.m_pvec[i];
      }
    }
    void operator/=(const NDPoint<ND, T>& a_pt)
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
    void operator-=(const NDPoint<ND, T>& a_pt)
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
    bool operator==(const NDPoint<ND, T>& a_pt)
    {
      for (int i = 0; i < ND; i++)
      {
        if (m_pvec[i] != a_pt.m_pvec[i])
          return false;
      }
      return true;
    }
    /**
     * Checks that all components are less than their respective counterparts in a_pt.
     */
    bool operator<(const NDPoint<ND, T>& a_pt)
    {
      for (int i = 0; i < ND; i++)
      {
        if (m_pvec[i] >= a_pt.m_pvec[i])
          return false;
      }
      return true;
    }
    /**
     * Component accessor.
     */
    T& operator[](const int& index)
    {
      return m_pvec[index];
    }
    /**
     * Const component accessor.
     */
    const T& operator[](const int& index) const
    {
      return m_pvec[index];
    }
    /**
     * Computes the length of the vector defined by this point.
     */
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
    /**
     * Computes the distance between two points.
     */
    double distFrom(const NDPoint<ND, T>& a_pt) const
    {
      NDPoint<ND, T> pt = *this - a_pt;
      return pt.mag();
    }
    /**
     * Clamps all components to be between a_minpt and a_maxpt.
     */
    void clamp(const NDPoint<ND, T>& a_minpt, const NDPoint<ND, T>& a_maxpt)
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

  /**
   * Returns a point with all zero components
   */
  template <int ND, typename T = double>
  NDPoint<ND, T> getZeros()
  {
    T zeros[ND];
    std::fill_n(zeros, ND, 0.0);
    return NDPoint<ND>(zeros);
  }

  /**
   * Returns a point with all components set to one.
   */
  template <int ND, typename T = double>
  NDPoint<ND, T> getOnes()
  {
    T ones[ND];
    std::fill_n(ones, ND, 1.0);
    return NDPoint<ND, T>(ones);
  }

  /**
   * Returns a point that defines the unit vector for the specified dimension.
   */
  template <int ND, typename T = double>
  NDPoint<ND, T> getUnitv(int a_dir)
  {
//    _ASSERT(a_dir >= 0 && a_dir < ND);
    T pvec[ND];
    std::fill_n(pvec, ND, 0.0);
    pvec[a_dir] = 1.0;
    return NDPoint<ND, T>(pvec);
  }
}
#endif // __NDPoint__