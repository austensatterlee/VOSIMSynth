/**
 *	 \file UnitControlContainer.h
 *   \brief
 *   \details
 *   \author Austen Satterlee
 *   \date 02/15/2016
 */
#pragma once
#include "UnitControl.h"
namespace syn {

	struct UnitPortVector {
		enum {
			Null = 0,
			Input,
			Output
		} type;
		int id;
	};

	class UnitControlContainer {
	public:
		UnitControlContainer(IPlugBase* a_plug, shared_ptr<VoiceManager> a_vm, UnitControl* a_unitControl, int a_unitId, int x, int y);

		virtual ~UnitControlContainer() {
			delete m_unitControl;
		};

		bool draw(IGraphics* pGraphics);

		void onMouseDblClick(int a_x, int a_y, IMouseMod* a_mouseMod);

		void onMouseDown(int a_x, int a_y, IMouseMod* a_mouseMod);

		void onMouseUp(int a_x, int a_y, IMouseMod* a_mouseMod);

		void onMouseDrag(int a_x, int a_y, int a_dX, int a_dY, IMouseMod* a_mouseMod);

		void onMouseOver(int a_x, int a_y, IMouseMod* a_mouseMod);

		virtual void onMouseWheel(int a_x, int a_y, IMouseMod* a_mouseMod, int a_d);

		void move(int a_newx, int a_newy);

		virtual NDPoint<2, int> getMinSize() const;

		void resize(NDPoint<2, int> a_newSize);

		NDPoint<2, int> getPos() const;

		NDPoint<2, int> getSize() const;

		virtual NDPoint<2, int> getPortPos(UnitPortVector a_portId) const;

		IRECT getPortRect(UnitPortVector a_portVector) const;

		virtual UnitPortVector getSelectedPort(int x, int y) const;

		void setUnitId(int a_newUnitId);

		int getUnitId() const;

		bool isHit(int a_x, int a_y);
	private:
		virtual void _resetMinSize();
		void _updateMinSize(NDPoint<2, int> a_newMinSize);
	private:
		friend class CircuitPanel;

		shared_ptr<VoiceManager> m_voiceManager;
		UnitControl* m_unitControl;

		int m_unitId;
		NDPoint<2, int> m_size;
		NDPoint<2, int> m_minSize;
		NDPoint<2, int> m_portSize;
		NDPoint<2, int> m_titleSize;
		const int c_portPad = 3;
		const int c_edgePad = 1;
		IRECT m_rect;
		IPlugBase* m_plug;
		bool m_isControlClicked;
	};
}