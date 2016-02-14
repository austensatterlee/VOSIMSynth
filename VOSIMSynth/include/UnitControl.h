#pragma once

#include "IControl.h"
#include "UnitParameter.h"
#include "NDPoint.h"
#include "ITextSlider.h"
#include <DSPMath.h>

namespace syn {
    class Unit;

    class VoiceManager;

    class CircuitPanel;

    struct UnitPortVector {
        enum {
            Null = 0,
            Input,
            Output
        } type;
        int id;
    };

    class UnitControl : public IControl {
    public:
        UnitControl(IPlugBase* a_plug, shared_ptr<VoiceManager> a_vm, int a_unitId, int x, int y);

        virtual ~UnitControl();

        virtual bool Draw(IGraphics* pGraphics) override;

        virtual void OnMouseDblClick(int x, int y, IMouseMod* pMod) override;

        virtual void OnMouseDown(int x, int y, IMouseMod* pMod) override;

        virtual void OnMouseUp(int x, int y, IMouseMod* pMod) override;

        void move(int a_newx, int a_newy);

		NDPoint<2, int> getMinSize() const;

        void resize(NDPoint<2,int> a_newSize);

        NDPoint<2, int> getPos() const;

		NDPoint<2, int> getSize() const;

        NDPoint<2, int> getPortPos(UnitPortVector a_portId) const;

        IRECT getPortRect(UnitPortVector a_portVector) const;

        UnitPortVector getSelectedPort(int x, int y) const;

        int getSelectedParam(int x, int y);

		void setUnitId(int a_newUnitId);
		int getUnitId() const;
    private:
        void _updateMinSize(NDPoint<2,int> minsize);
        void _resetMinSize();
    private:
        friend class CircuitPanel;

        shared_ptr<VoiceManager> m_voiceManager;

        int m_unitId;
		NDPoint<2, int> m_size;
		NDPoint<2, int> m_minSize;
        int m_x, m_y;
		NDPoint<2, int> c_portSize = { 10,10 };
        vector<ITextSlider> m_paramControls;
    };
}

