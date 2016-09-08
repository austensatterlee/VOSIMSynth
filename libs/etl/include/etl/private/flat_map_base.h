///\file

/******************************************************************************
The MIT License(MIT)

Embedded Template Library.
https://github.com/ETLCPP/etl
http://www.etlcpp.com

Copyright(c) 2015 jwellbelove

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
******************************************************************************/

#ifndef __ETL_IN_IFLAT_MAP_H__
#error This header is a private element of etl::flat_map & etl::iflat_map
#endif

#ifndef __ETL_FLAT_MAP_BASE__
#define __ETL_FLAT_MAP_BASE__

#include <stddef.h>

#include "../exception.h"
#include "../ivector.h"
#include "../error_handler.h"

#undef ETL_FILE
#define ETL_FILE "2"

namespace etl
{
  //***************************************************************************
  ///\ingroup flat_map
  /// Exception base for flat_maps
  //***************************************************************************
  class flat_map_exception : public exception
  {
  public:

    flat_map_exception(string_type what, string_type file_name, numeric_type line_number)
      : exception(what, file_name, line_number)
    {
    }
  };

  //***************************************************************************
  ///\ingroup flat_map
  /// Vector full exception.
  //***************************************************************************
  class flat_map_full : public flat_map_exception
  {
  public:

    flat_map_full(string_type file_name, numeric_type line_number)
      : flat_map_exception(ETL_ERROR_TEXT("flat_map: full", ETL_FILE"A"), file_name, line_number)
    {
    }
  };

  //***************************************************************************
  ///\ingroup flat_map
  /// Vector out of bounds exception.
  //***************************************************************************
  class flat_map_out_of_bounds : public flat_map_exception
  {
  public:

    flat_map_out_of_bounds(string_type file_name, numeric_type line_number)
      : flat_map_exception(ETL_ERROR_TEXT("flat_map:bounds", ETL_FILE"B"), file_name, line_number)
    {
    }
  };

  //***************************************************************************
  ///\ingroup flat_map
  /// The base class for all templated flat_map types.
  //***************************************************************************
  class flat_map_base
  {
  public:

    typedef size_t size_type;

    //*************************************************************************
    /// Gets the current size of the flat_map.
    ///\return The current size of the flat_map.
    //*************************************************************************
    size_type size() const
    {
      return vbase.size();
    }

    //*************************************************************************
    /// Checks the 'empty' state of the flat_map.
    ///\return <b>true</b> if empty.
    //*************************************************************************
    bool empty() const
    {
      return vbase.empty();
    }

    //*************************************************************************
    /// Checks the 'full' state of the flat_map.
    ///\return <b>true</b> if full.
    //*************************************************************************
    bool full() const
    {
      return vbase.full();
    }

    //*************************************************************************
    /// Returns the capacity of the flat_map.
    ///\return The capacity of the flat_map.
    //*************************************************************************
    size_type capacity() const
    {
      return vbase.capacity();
    }

    //*************************************************************************
    /// Returns the maximum possible size of the flat_map.
    ///\return The maximum size of the flat_map.
    //*************************************************************************
    size_type max_size() const
    {
      return vbase.max_size();
    }

    //*************************************************************************
    /// Returns the remaining capacity.
    ///\return The remaining capacity.
    //*************************************************************************
    size_t available() const
    {
      return vbase.available();
    }

  protected:

    //*************************************************************************
    /// Constructor.
    //*************************************************************************
    flat_map_base(vector_base& vbase)
      : vbase(vbase)
    {
    }

    vector_base& vbase;
  };
}

#undef ETL_FILE

#endif
