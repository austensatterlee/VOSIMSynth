#ifndef __UI__
#define __UI__
#include "IControl.h"
#include "IParam.h"
#include "Unit.h"
#include "NDPoint.h"
#include <vector>
#define SCREEN_PAD_L 25
#define X_PAD 10
#define Y_PAD 32
#define CELL_SIZE 32

using namespace std;
namespace syn
{

  inline NDPoint<4> hex2pt(int word)
  {
    int a = word >> 24;
    int r = (word >> 16) & 0xFF;
    int g = (word >> 8) & 0xFF;
    int b = word & 0xFF;
    return NDPoint<4>((double)a, (double)r, (double)g, (double)b);
  }

  inline IColor pt2color(const NDPoint<4>& pt)
  {
    return IColor(pt[0],pt[1],pt[2],pt[3]);
  }

  const vector<NDPoint<4>> palette = {
    hex2pt(0xFF06070E),
    hex2pt(0xFF29524A),
    hex2pt(0xFF94A187),
    hex2pt(0xFFC5AFA0),
    hex2pt(0xFFE9BCB7)
  };

  class Grid
  {
  private:
    struct Cell
    {
      int freerows;
      int freecols;
    };
    IGraphics& m_graphics;
    vector<vector<Cell> > m_cells;
    int m_rows, m_cols;
    int m_cellsize;
  public:
    Grid(IGraphics* pGraphics, int cellsize) :
    m_graphics(*pGraphics),
    m_cellsize(cellsize)
    {
      int height = pGraphics->Height();
      int width = pGraphics->Width();
      m_rows = height/cellsize;
      m_cols = width/cellsize;
      m_cells.resize(m_rows);
      for (int i = 0; i < m_rows; i++)
      {
        m_cells[i].resize(m_cols,{ m_rows,m_cols });
      }
    };
    IRECT place(const IRECT& size, int direction);
    void unplace(IRECT position);
    ~Grid() {};
  };

  class Panel
  {
  private:
    vector<double> points;
  public:
    Panel() {};
    ~Panel() {};
  };

  template<class U>
  void attachKnob(IGraphics *pGraphics, IPlugBase *pPlug, uint8_t r, uint8_t c, int paramIdx, string name, IBitmap *pBmp)
  {
    DBGMSG("%d x %d", pBmp->W, pBmp->H / pBmp->N);
    int x = (CELL_SIZE + X_PAD)*(c)+X_PAD + SCREEN_PAD_L;
    int y = (CELL_SIZE + Y_PAD)*(r)+Y_PAD;
    IText textprops(12, &COLOR_WHITE, "Helvetica");
    IParam* param = pPlug->GetParam(paramIdx);
    IRECT lbltextrect = IRECT(x, y + CELL_SIZE, x + CELL_SIZE, y + CELL_SIZE + Y_PAD / 2);
    IRECT valtextrect = IRECT(x, y + CELL_SIZE + Y_PAD / 2, x + CELL_SIZE, y + CELL_SIZE + Y_PAD);
    ICaptionControl* knobTextCtrl = new ICaptionControl(pPlug, lbltextrect, paramIdx, &textprops, false);
    ITextControl* knobLbl = new ITextControl(pPlug, valtextrect, &textprops, name.c_str());
    knobTextCtrl->DisablePrompt(false);
    U* knobCtrl = new U(pPlug, x, y, paramIdx, pBmp);

    pGraphics->AttachControl(knobCtrl);
    pGraphics->AttachControl(knobLbl);
    pGraphics->AttachControl(knobTextCtrl);
  }
  
}
#endif