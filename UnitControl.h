#pragma once
#include "IControl.h"
#include "UnitParameter.h"
#include "NDPoint.h"
#include "ITextSlider.h"

namespace syn {
  class Unit;
  class VoiceManager;

  struct SelectedPort
  {
    int paramid;
    MOD_ACTION modaction;
  };

  class CircuitPanel;

  class UnitControl : public IControl
  {
  private:
    friend class CircuitPanel;
  public:
    UnitControl(IPlugBase* pPlug, VoiceManager* vm, Unit* unit, int x, int y, int size = 10);
    virtual ~UnitControl();
    virtual bool Draw(IGraphics* pGraphics) override;
    virtual void OnMouseDblClick(int x, int y, IMouseMod* pMod) override;
    virtual void OnMouseDown(int x, int y, IMouseMod* pMod) override;
    virtual void OnMouseUp(int x, int y, IMouseMod* pMod) override;
    void move(int newx, int newy);
    int getMinSize() const;
    void resize(int newsize);
    NDPoint<2, int> getPos() const;
    NDPoint<2, int> getPortPos(SelectedPort& port);
    NDPoint<2, int> getOutputPos() const;
    Unit* getUnit() const;
    SelectedPort getSelectedPort(int x, int y);
    int getSelectedParam(int x, int y);
  private:
    struct Port
    {
      IRECT add_rect;
      IRECT scale_rect;
    };

  protected:
    void updateMinSize(int minsize);
    Unit* m_unit;
    int m_size;
    int m_minsize;
    const size_t m_nParams;
    int m_x, m_y;
    bool m_is_sink;
    vector<ITextSlider> m_portLabels;
    vector<Port> m_ports;
  };
}

