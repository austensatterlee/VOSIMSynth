/**  
 *	 \file OscilloscopeUnit.h
 *   \brief 
 *   \details
 *   \author Austen Satterlee
 *   \date 02/15/2016
 */
#pragma once
#include <Unit.h>
#include <UnitControl.h>
#include <DSPMath.h>

namespace syn {
	class OscilloscopeUnit : public Unit
	{	
	public:
		OscilloscopeUnit(const string& a_name) :
			Unit(a_name),
			m_bufferIndex(0),
			m_pBufferSize(addParameter_(UnitParameter("buffer size", 2, 1024, 256))),
			m_nBuffers(2)
		{
			addInput_("in1");
			addInput_("in2");
			m_buffer.resize(m_nBuffers);			
		}

		OscilloscopeUnit(const OscilloscopeUnit& a_rhs) :
			OscilloscopeUnit(a_rhs.getName())
		{}
	protected:
		void onParamChange_(int a_paramId) override;
		void process_(const SignalBus& a_inputs, SignalBus& a_outputs) override;
		
	private:
		inline string _getClassName() const override { return "OscilloscopeUnit"; };
		Unit* _clone() const override { return new OscilloscopeUnit(*this);  };
	private:
		friend class OscilloscopeUnitControl;
		int m_bufferIndex;
		int m_bufferSize;
		int m_nBuffers;
		int m_pBufferSize;
		vector<vector<double> > m_buffer;
	};

	class OscilloscopeUnitControl : public UnitControl
	{
	public:
		OscilloscopeUnitControl() :
			UnitControl(),
			m_yBounds(-1.0, 1.0),
			m_paramControl(nullptr)
		{}

		OscilloscopeUnitControl(IPlugBase* a_plug, const shared_ptr<VoiceManager>& a_vm, int a_unitId, int a_x, int a_y) : 
			UnitControl(a_plug, a_vm, a_unitId, a_x, a_y),
			m_yBounds(-1.0, 1.0),
			m_paramControl(new DefaultUnitControl(a_plug, a_vm, a_unitId, a_x, a_y)) 
		{}


		virtual ~OscilloscopeUnitControl() {
			if(m_paramControl!=nullptr)
				delete m_paramControl;
		}

		void onMouseDblClick(int a_x, int a_y, IMouseMod* a_mouseMod) override {
			m_paramControl->onMouseDblClick(a_x, a_y, a_mouseMod);
		}
		void onMouseDown(int a_x, int a_y, IMouseMod* a_mouseMod) override {
			m_paramControl->onMouseDown(a_x, a_y, a_mouseMod);
		}
		void onMouseUp(int a_x, int a_y, IMouseMod* a_mouseMod) override {
			m_paramControl->onMouseUp(a_x, a_y, a_mouseMod);
		}
		void onMouseDrag(int a_x, int a_y, int a_dX, int a_dY, IMouseMod* a_mouseMod) override {
			m_paramControl->onMouseDrag(a_x, a_y, a_dX, a_dY, a_mouseMod);
		}

		void draw(IGraphics* a_graphics) override 
		{
			// Draw background
			IRECT rect = { m_pos[0], m_pos[1], m_pos[0] + m_size[0], m_pos[1] + m_size[1] };
			a_graphics->FillIRect(&COLOR_BLACK, &rect);			
			m_paramControl->draw(a_graphics);
			updateMinSize_(m_paramControl->getMinSize());
			NDPoint<2,int> paramControlSize = m_paramControl->getMinSize();			

			IRECT screen_rect = { rect.L,rect.T + paramControlSize[1],rect.R,rect.B };
			const OscilloscopeUnit& unit = static_cast<const OscilloscopeUnit&>(m_voiceManager->getUnit(m_unitId));
			NDPoint<2, int> lastPoint(0,0);
			NDPoint<2, int> currPoint;
			for (int i = 0; i < unit.m_nBuffers; i++) {
				for (int j = 0; j < unit.m_buffer[i].size(); j++) {
					currPoint = toScreen({ static_cast<double>(j),unit.m_buffer[i][j] }, screen_rect);
					if (j>0)
						a_graphics->DrawLine(&m_colors[i], static_cast<float>(lastPoint[0]), static_cast<float>(lastPoint[1]), static_cast<float>(currPoint[0]), static_cast<float>(currPoint[1]), 0, true);
					lastPoint = currPoint;
				}
			}
		}
		int getSelectedParam(int a_x, int a_y) override {
			return m_paramControl->getSelectedParam(a_x, a_y);
		}
	protected:
		void onSetUnitId_() override{
			m_paramControl->setUnitId(m_unitId);
		};
		void onChangeRect_() override {
			m_paramControl->move(m_pos);
			m_paramControl->resize({ m_size[0]-1,0 });
		}
	private:
		UnitControl* _construct(IPlugBase* a_plug, shared_ptr<VoiceManager> a_vm, int a_unitId, int a_x, int a_y) const override 
		{
			return new OscilloscopeUnitControl(a_plug, a_vm, a_unitId, a_x, a_y);
		};

		NDPoint<2,int> toScreen(NDPoint<2,double> a_pt, const IRECT& a_screen) {
			const OscilloscopeUnit& unit = static_cast<const OscilloscopeUnit&>(m_voiceManager->getUnit(m_unitId));
			NDPoint<2, double> xBounds(0, static_cast<double>(unit.m_bufferSize));
			a_pt[0] = CLAMP(a_pt[0], xBounds[0], xBounds[1]);
			a_pt[1] = CLAMP(a_pt[1], m_yBounds[0], m_yBounds[1]);
			
			NDPoint<2, int> screenPt;
			screenPt[0] = (a_pt[0] - xBounds[0]) / (xBounds[1] - xBounds[0]) * a_screen.W() + a_screen.L;
			screenPt[1] = (-a_pt[1] - m_yBounds[0]) / (m_yBounds[1] - m_yBounds[0]) * a_screen.H() + a_screen.T;
			return screenPt;
		}
	private:
		NDPoint<2, double> m_yBounds;
		DefaultUnitControl* m_paramControl;
		IColor m_colors[2] = { {255,255,255,255},{255,128,128,255} };
	};
}
