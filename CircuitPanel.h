#pragma once
#include "IControl.h"
#include "ITextSlider.h"
#include "NDPoint.h"
#include "UnitFactory.h"
#include "Containers.h"
#include "UnitControl.h"
#include <unordered_map>
#include <memory>
#include <array>

#define WIRE_THRESH 10

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
		array<int, 3> m_nearestWire;
		void updateInstrument() const;
		void deleteUnit(int unitctrlid);
		void deleteWire(int unitctrlid, int connection_idx);
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
			m_nearestWire{0,0,-1},
			IControl(pPlug, pR) { };

		~CircuitPanel() { };

		void OnMouseOver(int x, int y, IMouseMod* pMod) override;
		void OnMouseDown(int x, int y, IMouseMod* pMod) override;
		void createUnit(string proto_name, int x, int y);
		void OnMouseUp(int x, int y, IMouseMod* pMod) override;
		void OnMouseDrag(int x, int y, int dX, int dY, IMouseMod* pMod) override;
		void OnMouseDblClick(int x, int y, IMouseMod* pMod) override;
		bool Draw(IGraphics* pGraphics) override;
		int getSelectedUnit(int x, int y); 
		bool IsDirty() override { return true; }
		array<int, 3> getNearestWire(int x, int y); /// \returns an array containing the unit control index, port index, and the euclidean distance from the wire.

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
		 * Generate the context menu used to build new units
		 */
		void generateUnitFactoryMenu(shared_ptr<IPopupMenu> main_menu, vector<shared_ptr<IPopupMenu>>& sub_menus) const
		{
			const set<string>& group_names = m_unitFactory->getGroupNames();
			int i = 0;
			for (const string& group_name : group_names) {
				while (sub_menus.size() <= i)
					sub_menus.push_back(make_shared<IPopupMenu>());
				main_menu->AddItem(group_name.c_str(), sub_menus[i].get());
				const vector<string>& proto_names = m_unitFactory->getPrototypeNames(group_name);
				int j = 0;
				for (const string& proto_name : proto_names) {
					sub_menus[i]->AddItem(proto_name.c_str(), j);
					j++;
				}
				i++;
			}
		}

		/**
		 * Determines whether the mouse is close enough to interact with a wire
		 */
		bool checkNearestWire() const
		{
			return m_nearestWire[2] >= 0 && m_nearestWire[2] < WIRE_THRESH;
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

