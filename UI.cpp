#include "UI.h"
#include "UnitParameter.h"
namespace syn
{

  IRECT Grid::place(const IRECT& size, int direction)
  {
    int reqRows = std::ceil(size.H() / (double)m_cellsize);
    int reqCols = std::ceil(size.W() / (double)m_cellsize);
    int i = 0;
    int j = 0;
    while (i < m_rows)
    {
      while (j < m_cols)
      {
        Cell& cell = m_cells[i][j];
        if (cell.freecols >= reqCols && cell.freerows >= reqRows)
        {
          cell.freecols -= reqCols;
          cell.freerows -= reqRows;          
        }
        j += cell.freecols;        
      }
      i++;
    }
    return IRECT(0,0,0,0);
  }

  void Grid::unplace(IRECT position)
  {

  }

}