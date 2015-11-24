#pragma once
#include "IControl.h"
#include "ITextSlider.h"
#include "NDPoint.h"
#include "UnitFactory.h"
#include <unordered_map>
using namespace std;

namespace syn
{
  struct SelectedPort
  {
    int paramid;
    MOD_ACTION modaction;
  };

  class UnitControl : public IControl
  {
  public:
    UnitControl(IPlugBase* pPlug, VoiceManager* vm, Unit* unit, int x, int y, int size = 10);
    virtual ~UnitControl();
    virtual bool Draw(IGraphics* pGraphics) override;
    void move(int newx, int newy);
    void resize(int newsize);
    NDPoint<2, int> getPos();
    NDPoint<2, int> getPortPos(SelectedPort& port);
    NDPoint<2, int> getOutputPos();
    Unit* getUnit();
    SelectedPort getSelectedPort(int x, int y);
    int getSelectedParam(int x, int y);
    void modParam(int paramid, double dX);
  private:
    struct Port
    {
      IRECT add_rect;
      IRECT scale_rect;
    };
  protected:
    Unit* m_unit;
    int m_size;
    int m_nParams;
    int m_x, m_y;
    bool m_is_sink;
    vector<ITextSlider> m_portLabels;
    vector<Port> m_ports;
  };
  class CircuitPanel : public IControl
  {
  protected:
    enum DRAG_ACTION
    {
      NONE = 0,
      MOVE,
      RESIZE,
      MOD_PARAM,
      CONNECT
    };
    unordered_map<int, UnitControl*> m_unitControls;
    VoiceManager* m_vm;
    UnitFactory* m_unitFactory;
    IPopupMenu m_main_menu;
    IPopupMenu m_unit_menu;
    IPopupMenu m_sourceunit_menu;
    int m_isMouseDown;
    NDPoint<2, int> m_lastMousePos, m_lastClickPos;
    DRAG_ACTION m_currAction;
    int m_lastSelectedUnit, m_lastSelectedParam;
    SelectedPort m_lastSelectedPort;
    void updateInstrument();
    void deleteUnit(int unitctrlid);
  public:
    CircuitPanel(IPlugBase* pPlug, IRECT pR, VoiceManager* voiceManager, UnitFactory* unitFactory) :
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
    ~CircuitPanel() {};

    virtual void OnMouseDown(int x, int y, IMouseMod* pMod) override;

    virtual void OnMouseUp(int x, int y, IMouseMod* pMod) override;

    virtual void OnMouseDrag(int x, int y, int dX, int dY, IMouseMod* pMod) override;

    virtual void OnMouseDblClick(int x, int y, IMouseMod* pMod) override;

    virtual bool Draw(IGraphics* pGraphics) override;

    int getSelectedUnit(int x, int y);

    virtual bool IsDirty() override { return true; }

  };


}