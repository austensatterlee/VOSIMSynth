#ifndef __UI__
#define __UI__
#include "IControl.h"
#include "IParam.h"
#include "Unit.h"
#include "NDPoint.h"
#include "VoiceManager.h"
#include "UnitFactory.h"
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

  const vector<ColorPoint> palette = {
    {0xFF06070E},
    {0xFF29524A},
    {0xFF94A187},
    {0xFFC5AFA0},
    {0xFFE9BCB7}
  };

  class UnitControl : public IControl
  {
  public:
    UnitControl(IPlugBase* pPlug, Unit* unit, int x, int y, int size = 10);
    virtual ~UnitControl();
    virtual bool Draw(IGraphics* pGraphics) override;
    void move(int newx, int newy);
    void resize(int newsize);
    NDPoint<2, int> getPos();
    NDPoint<2, int> getPortPos(int port);
    NDPoint<2, int> getOutputPos();
    Unit* getUnit();
    int getSelectedParam(int x, int y);
  protected:
    Unit* m_unit;
    int m_size;
    int m_nParams;
    int m_x, m_y;
    bool m_is_sink;
    vector<IRECT> m_portLabels;
    vector<IRECT> m_ports;
  };

  class UnitPanel : public IControl
  {
  protected:
    enum DRAG_ACTION
    {
      NONE = 0,
      MOVE,
      RESIZE,
      CONNECT
    };
    unordered_map<int,UnitControl*> m_unitControls;
    VoiceManager* m_vm;
    UnitFactory* m_unitFactory;
    IPopupMenu m_main_menu;
    IPopupMenu m_unit_menu;
    IPopupMenu m_sourceunit_menu;
    int m_isMouseDown;
    NDPoint<2, int> m_lastMousePos, m_lastClickPos;
    DRAG_ACTION m_currAction;
    int m_lastSelectedUnit, m_lastSelectedParam;
    void updateInstrument();
    void deleteUnit(int unitctrlid);
  public:
    UnitPanel(IPlugBase* pPlug, IRECT pR, VoiceManager* voiceManager, UnitFactory* unitFactory) :
      m_vm(voiceManager),
      m_unitFactory(unitFactory),
      m_isMouseDown(0),
      m_lastSelectedUnit(-1),
      m_lastMousePos(0, 0),
      m_lastClickPos(0, 0),
      IControl(pPlug, pR)
    {
      m_main_menu.AddItem("Source Units", &m_sourceunit_menu);
      m_main_menu.AddItem("Units", &m_unit_menu);
      const vector<string>& unitNames = unitFactory->getPrototypeNames();
      for (int i = 0; i < unitNames.size(); i++)
      {
        m_unit_menu.AddItem(unitNames[i].c_str(), i);
      }
      const vector<string>& srcUnitNames = unitFactory->getSourcePrototypeNames();
      for (int i = 0; i < srcUnitNames.size(); i++)
      {
        m_sourceunit_menu.AddItem(srcUnitNames[i].c_str(), i);
      }
    };
    ~UnitPanel() {};

    virtual void OnMouseDown(int x, int y, IMouseMod* pMod) override;

    virtual void OnMouseUp(int x, int y, IMouseMod* pMod) override;

    virtual void OnMouseDrag(int x, int y, int dX, int dY, IMouseMod* pMod) override;

    virtual void OnMouseDblClick(int x, int y, IMouseMod* pMod) override;

    virtual bool Draw(IGraphics* pGraphics) override;

    int getSelectedUnit(int x, int y);

    virtual bool IsDirty() override { return true; }

  };

  template<class U>
  void attachKnob(IGraphics *pGraphics, IPlugBase *pPlug, uint8_t r, uint8_t c, int paramIdx, string name, IBitmap *pBmp)
  {
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