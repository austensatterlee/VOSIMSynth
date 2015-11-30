#pragma once
#include "IControl.h"
#include "ITextSlider.h"
#include "NDPoint.h"
#include "UnitFactory.h"
#include "Containers.h"
#include <unordered_map>
using namespace std;

namespace syn
{
  struct SelectedPort
  {
    int paramid;
    MOD_ACTION modaction;
  };

  class CircuitPanel;

  class UnitControl : public IControl
  {
    friend class CircuitPanel;
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
    const size_t m_nParams;
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
    void setSink(int unitctrlid);
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
    void createSourceUnit(int factoryid, int x, int y);
    void createUnit(int factoryid, int x, int y);
    virtual void OnMouseUp(int x, int y, IMouseMod* pMod) override;
    virtual void OnMouseDrag(int x, int y, int dX, int dY, IMouseMod* pMod) override;
    virtual void OnMouseDblClick(int x, int y, IMouseMod* pMod) override;
    virtual bool Draw(IGraphics* pGraphics) override;
    int getSelectedUnit(int x, int y);
    virtual bool IsDirty() override { return true; }

    /**
    * Serializes the current state of the CircuitPanel, including instrument topology and parameters
    *
    * Structure:
    *  - [unsigned integer] N     - number of units
    *  - UNIT - unit serialization (See CircuitPanel::serializeUnitControl and UnitControl::unserializeUnitControl)
    *  - ...
    *  - UNIT [N]
    *
    *  - ConnectionBlock:
    *   + [unsigned integer] M - number of connections coming into the unit associated with UnitControl[0]
    *   + [ConnectionMetadata] CONNECTION
    *   + ...
    *   + [ConnectionMetadata] CONNECTION[M]
    *  - ...
    *  - ConnectionBlock[N]
    *
    */
    ByteChunk serialize() const;
    /**
     * Configures the CircuitPanel to match the given serialization.
     * Note that this will change the composition of the prototype instrument.
     */
    int unserialize(ByteChunk* serialized, int startPos);
  private:
    /**
     * Serializes the specified UnitControl, including the associated Unit itself
     */
    ByteChunk serializeUnitControl(int ctrlidx) const;
    /**
     * Restores the UnitControl specified in the provided serialization.
     * The associated Unit is created and added to the prototype instrument.     *
     * Note that the UnitControl is restored using its original control id, so it may deallocate and overwrite an existing unit.
     */
    int unserializeUnitControl(ByteChunk* chunk, int startPos);
  };
}