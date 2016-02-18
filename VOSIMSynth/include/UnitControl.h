#pragma once

#include "IControl.h"
#include "UnitParameter.h"
#include "NDPoint.h"
#include "ITextSlider.h"

namespace syn {

	class UnitControl
	{
	public:
		UnitControl();
		UnitControl(IPlugBase* a_plug, shared_ptr<VoiceManager> a_vm, int a_unitId, int a_x, int a_y);

		virtual ~UnitControl() {}

		virtual void draw(IGraphics* a_graphics) = 0;

		virtual void onMouseDblClick(int a_x, int a_y, IMouseMod* a_mouseMod) {};

		virtual void onMouseDown(int a_x, int a_y, IMouseMod* a_mouseMod) {};

		virtual void onMouseUp(int a_x, int a_y, IMouseMod* a_mouseMod) {};

		virtual void onMouseDrag(int a_x, int a_y, int a_dX, int a_dY, IMouseMod* a_mouseMod) {};

		virtual void onMouseWheel(int a_x, int a_y, IMouseMod* a_mouseMod, int a_d) {};

		bool isHit(int a_x, int a_y) const;

		void move(NDPoint<2, int> a_newPos);

		UnitControl* construct(IPlugBase* a_mPlug, shared_ptr<VoiceManager> a_voiceManager, int a_uid, int a_x, int a_y) const;

		void resize(NDPoint<2, int> a_newSize);

		NDPoint<2, int> getMinSize() const;

		void setUnitId(int a_newUnitId);

		virtual int getSelectedParam(int a_x, int a_y);
	protected:
		void updateMinSize_(NDPoint<2, int> a_newMinSize);

		void resetMinSize_();

		virtual void onSetUnitId_() {};
		virtual void onChangeRect_() {};
	protected:
		shared_ptr<VoiceManager> m_voiceManager;
		int m_unitId;
		NDPoint<2, int> m_pos;
		NDPoint<2, int> m_size;
		IPlugBase* m_plug;
	private:
		virtual UnitControl* _construct(IPlugBase* a_plug, shared_ptr<VoiceManager> a_vm, int a_unitId, int a_x, int a_y) const = 0;
	private:
		NDPoint<2, int> m_minSize;
	};

	class DefaultUnitControl : public UnitControl
	{		
	public:
		DefaultUnitControl(IPlugBase* a_plug, shared_ptr<VoiceManager> a_vm, int a_unitId, int a_x, int a_y) :
			UnitControl(a_plug, a_vm, a_unitId, a_x, a_y),
			m_lastSelectedParam(-1)
		{
			int nParams = m_voiceManager->getUnit(m_unitId).getNumParameters();
			m_paramControls.clear();
			for (int i = 0; i < nParams; i++) {
				m_paramControls.push_back(ITextSlider(m_plug, m_voiceManager, m_unitId, i, IRECT{ 0, 0, 0, 0 }));
			}
		}

		void draw(IGraphics* a_graphics) override {
			int nParams = m_paramControls.size();
			resetMinSize_();
			// Draw user controls
			for (int i = 0; i < nParams; i++) {
				m_paramControls[i].Draw(a_graphics);
				updateMinSize_({ m_paramControls[i].getMinSize(), 0 });
			}
			updateMinSize_({ 0, 10*nParams });
		}

		void onMouseDblClick(int a_x, int a_y, IMouseMod* a_mouseMod) override {
			int selectedParam = getSelectedParam(a_x, a_y);
			if (selectedParam >= 0) {
				m_paramControls[selectedParam].OnMouseDblClick(a_x, a_y, a_mouseMod);
			}
		}

		void onMouseDown(int a_x, int a_y, IMouseMod* a_mouseMod) override {
			int selectedParam = getSelectedParam(a_x, a_y);
			if (selectedParam >= 0) {
				m_paramControls[selectedParam].OnMouseDown(a_x, a_y, a_mouseMod);
				m_lastSelectedParam = selectedParam;
			}
		}

		void onMouseUp(int a_x, int a_y, IMouseMod* a_mouseMod) override {
			if (m_lastSelectedParam >= 0) {
				m_paramControls[m_lastSelectedParam].OnMouseUp(a_x, a_y, a_mouseMod);
			}
			m_lastSelectedParam = -1;
		}

		void onMouseDrag(int a_x, int a_y, int a_dX, int a_dY, IMouseMod* a_mouseMod) override {
			if (m_lastSelectedParam >= 0) {
				m_paramControls[m_lastSelectedParam].OnMouseDrag(a_x, a_y, a_dX, a_dY, a_mouseMod);
			}
		}

		void onMouseWheel(int a_x, int a_y, IMouseMod* a_mouseMod, int a_d) override {
			int selectedParam = getSelectedParam(a_x, a_y);
			if (selectedParam >= 0) {
				m_paramControls[selectedParam].OnMouseWheel(a_x, a_y, a_mouseMod, a_d);
			}
		}

		int getSelectedParam(int a_x, int a_y) override {
			int selectedParam = -1;
			for (int i = 0; i < m_paramControls.size(); i++) {
				if (m_paramControls[i].GetRECT()->Contains(a_x, a_y)) {
					selectedParam = i;
				}
			}
			return selectedParam;
		}

	protected:

		void onSetUnitId_() override {
			m_paramControls.clear();
			int nParams = m_voiceManager->getUnit(m_unitId).getNumParameters();
			for (int i = 0; i < nParams; i++) {
				m_paramControls.push_back(ITextSlider(m_plug, m_voiceManager, m_unitId, i, IRECT{ 0, 0, 0, 0 }));
			}
		}

		void onChangeRect_() override {
			int nParams = m_paramControls.size();
			if (nParams) {
				int portY = m_pos[1];
				int rowsize = m_size[1] / nParams;
				for (int i = 0; i < nParams; i++) {
					IRECT param_rect{ m_pos[0], portY, m_pos[0] + m_size[0], portY + rowsize };
					m_paramControls[i].setRECT(param_rect);
					portY += rowsize;
				}
			}
		}
	private:
		UnitControl* _construct(IPlugBase* a_plug, shared_ptr<VoiceManager> a_vm, int a_unitId, int a_x, int a_y) const override {
			return new DefaultUnitControl(a_plug, a_vm, a_unitId, a_x, a_y);
		}
	private:
		vector<ITextSlider> m_paramControls;
		int m_lastSelectedParam;
	};

}

