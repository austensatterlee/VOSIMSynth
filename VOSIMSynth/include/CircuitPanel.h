/*
Copyright 2016, Austen Satterlee

This file is part of VOSIMProject.

VOSIMProject is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

VOSIMProject is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with VOSIMProject. If not, see <http://www.gnu.org/licenses/>.
*/

/**
* \file CircuitPanel.h
* \brief
* \details
* \author Austen Satterlee
* \date March 6, 2016
*/
#ifndef __CIRCUITPANEL__
#define __CIRCUITPANEL__
  
#include "IControl.h"
#include "ITextSlider.h"
#include "NDPoint.h"
#include "UnitFactory.h"
#include "Containers.h"
#include "UnitControl.h"
#include "UnitControlContainer.h"
#include "DefaultUnitControl.h"
#include <memory>
#include <functional>

#define WIRE_THRESH 5

using namespace std;

namespace syn {

    struct CircuitPortVector {
        enum {
            Null = 0,
            Input,
            Output
        } type;
        int id;
    };

    class CircuitPanel : public IControl {
    public:

	    CircuitPanel(IPlugBase* pPlug, IRECT pR, VoiceManager* a_voiceManager, UnitFactory*
        a_factory) :
                IControl(pPlug, pR),
                m_voiceManager(a_voiceManager),
                m_unitFactory(a_factory),
                m_lastMousePos(0, 0),
                m_lastMouseState(0),
                m_lastClickPos(0, 0),
                m_lastClickState(0),
                m_currAction(NONE),
				m_lastClickedUnit(-1), 
				m_lastSelectedUnit(-1)
		{ };

        virtual ~CircuitPanel()
        { };
		
		typedef function<UnitControl*(IPlugBase*, VoiceManager*, int a_uid)> UnitControlConstructor;

		template <typename UnitType, typename UnitControlType>
	    void registerUnitControl() {
			unsigned classId = UnitType("").getClassIdentifier();
			m_unitControlMap[classId] = [](IPlugBase* a_plug, VoiceManager* a_vm, int a_uid)->UnitControl*
			{
				return UnitControl::construct<UnitControlType>(a_plug, a_vm, a_uid);
			};
		}

	    void OnMouseOver(int x, int y, IMouseMod* pMod) override;

        void OnMouseDown(int x, int y, IMouseMod* pMod) override;

        void OnMouseUp(int x, int y, IMouseMod* pMod) override;

        void OnMouseDrag(int x, int y, int dX, int dY, IMouseMod* pMod) override;

        void OnMouseDblClick(int x, int y, IMouseMod* pMod) override;

		void OnMouseWheel(int x, int y, IMouseMod* pMod, int d) override;

        bool Draw(IGraphics* pGraphics) override;
	
		bool IsDirty() override { return true; };

        int getSelectedUnit(int x, int y) const;

		vector<int> getUnitSelection(IRECT& a_rect);

		void clearUnitSelection();

		/**
		 * \returns true if unit is not already in selection, false otherwise
		 */
		bool addUnitToSelection(int a_unitid);

		/**
		* \returns true if unit was in selection, false otherwise
		*/
		bool removeUnitFromSelection(int a_unitId);

		bool isUnitInSelection(int a_unitId);

        NDPoint<2,int> getPortPos(const CircuitPortVector& a_vec) const;

        IRECT getPortRect(const CircuitPortVector& a_vec) const;

        CircuitPortVector getSelectedPort(int x, int y) const;

		/**
		* Serializes the current state of the CircuitPanel, including instrument topology and parameters
		*
		* Structure:
		*  - <unsigned integer> N - number of units
		*  - UNIT (xN) - unit serialization (See CircuitPanel::_serializeUnitControl and UnitControlContainer::_unserializeUnitControl)
		*
		*
		*  - <unsigned integer> M - number of connection records
		*  - <ConnectionRecord> (xM) (See ConnectionRecord)
		*
		*/
        ByteChunk serialize() const;

		/**
		 * Configures the CircuitPanel to match the given serialization.
		 * Note that this will change the composition of the prototype instrument.
		 */
        int unserialize(ByteChunk* serialized, int startPos);

	private:
		enum UIAction {
			NONE = 0,
			MODIFYING_UNIT,
			CONNECTING,
			SELECTING
		};
		typedef vector<shared_ptr<UnitControlContainer> > UnitList;
		VoiceManager* m_voiceManager;
		UnitFactory* m_unitFactory;
		map<unsigned, UnitControlConstructor> m_unitControlMap;
		UnitList m_unitControls;
		NDPoint<2, int> m_lastMousePos;
		IMouseMod m_lastMouseState;
		NDPoint<2, int> m_lastClickPos;
		IMouseMod m_lastClickState;
		ConnectionRecord m_newWire;

		UIAction m_currAction;
		vector<int> m_selectedUnits;
		int m_lastClickedUnit;
		int m_lastSelectedUnit;
		UnitPortVector m_lastClickedUnitPort;
		CircuitPortVector m_lastClickedCircuitPort;

		struct SelectedWire
		{
			ConnectionRecord record;
			int distFromWire;
			double distFromInputPort;
		} m_nearestWire = { { ConnectionRecord::Null,0,0,0,0 }, 0, 0.0 };

		const int c_portSize = 15;

    private:
        void _deleteUnit(int unitctrlid);

        void _deleteWire(ConnectionRecord a_conn) const;

        void _generateUnitFactoryMenu(shared_ptr<IPopupMenu> main_menu, vector<shared_ptr<IPopupMenu> >& sub_menus) const;

		/**
		 * Adds a new unit to the Circuit graphs on the audio thread, then adds the respective GUI elements.
		 */
		template<typename T>
	    int _createUnit(T a_prototypeId, const NDPoint<2,int>& a_pos);

	    bool _checkNearestWire() const;

		/**
		 * Serializes the specified UnitControlContainer, including the associated Unit itself
		 */
        ByteChunk _serializeUnitControl(int ctrlidx) const;

		/**
		 * Restores the UnitControlContainer specified in the provided serialization.
		 * The associated Unit is created and added to the prototype instrument.
		 */
        int _unserializeUnitControl(ByteChunk* chunk, int startPos);


		/**
		* \returns an array containing the port index, and the euclidean distance from the wire.
		*/
		SelectedWire _getNearestWire(int x, int y) const;
    };

	template <typename T>
	int CircuitPanel::_createUnit(T prototypeIdentifier, const NDPoint<2,int>& a_pos) {
		if(!m_unitFactory->hasClassId(prototypeIdentifier)) {
			return -1;
		}
		int uid = m_voiceManager->addUnit(prototypeIdentifier);

#ifndef NDEBUG
		if (m_unitControls.size() != uid)
			throw logic_error("unit control vector out of sync with circuit");
#endif

		unsigned classId = m_unitFactory->getClassId(prototypeIdentifier);
		UnitControl* unitCtrl;
		if (m_unitControlMap.find(classId) != m_unitControlMap.end()) {
			unitCtrl = m_unitControlMap[classId](mPlug, m_voiceManager, uid);
		} else {
			unitCtrl = UnitControl::construct<DefaultUnitControl>(mPlug, m_voiceManager, uid);
		}
		m_unitControls.push_back(make_shared<UnitControlContainer>(mPlug, m_voiceManager, unitCtrl, uid));
		m_unitControls.back()->move(a_pos);
		return m_unitControls.size() - 1;
	}
}


#endif
