#pragma once
#include "IControl.h"
#include "ITextSlider.h"
#include "NDPoint.h"
#include "UnitFactory.h"
#include "Containers.h"
#include "UnitControl.h"
#include <unordered_map>
#include <memory>

using namespace std;

namespace syn
{
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
    int m_isMouseDown;
    NDPoint<2, int> m_lastMousePos, m_lastClickPos;
    DRAG_ACTION m_currAction;
    int m_lastSelectedUnit, m_lastSelectedParam;
    SelectedPort m_lastSelectedPort;
    void updateInstrument() const;
    void deleteUnit(int unitctrlid);
    void setSink(int unitctrlid);
  public:
    CircuitPanel(IPlugBase* pPlug, IRECT pR, VoiceManager* voiceManager, UnitFactory* unitFactory) :
      m_vm(voiceManager),
      m_unitFactory(unitFactory),
      m_isMouseDown(0),
      m_lastSelectedUnit(-1),
      m_lastSelectedParam(-1),
      m_lastMousePos(0, 0),
      m_lastClickPos(0, 0),
      m_currAction(NONE),
      IControl(pPlug, pR)
    {
    };

    ~CircuitPanel()
    {
    };

    virtual void OnMouseDown(int x, int y, IMouseMod* pMod) override;
	  void createUnit(string proto_name, int x, int y);
    virtual void OnMouseUp(int x, int y, IMouseMod* pMod) override;
    virtual void OnMouseDrag(int x, int y, int dX, int dY, IMouseMod* pMod) override;
    virtual void OnMouseDblClick(int x, int y, IMouseMod* pMod) override;
    virtual bool Draw(IGraphics* pGraphics) override;
    int getSelectedUnit(int x, int y);

    virtual bool IsDirty() override
    {
      return true;
    }

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
	 void generateUnitFactoryMenu(shared_ptr<IPopupMenu> main_menu, vector<shared_ptr<IPopupMenu>>& sub_menus) const {
		 const set<string>& group_names = m_unitFactory->getGroupNames();
		 int i = 0;
		 for (const string& group_name : group_names) {
			 while (sub_menus.size() <= i)
				 sub_menus.push_back(make_shared<IPopupMenu>());
			 main_menu->AddItem(group_name.c_str(), sub_menus[i].get());
			 const vector<string>& proto_names = m_unitFactory->getPrototypeNames(group_name);
			 int j = 0;
			 for(const string& proto_name : proto_names) {				 
				 sub_menus[i]->AddItem(proto_name.c_str(), j);
				 j++;
			 }
			 i++;
		 }
	  }
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

