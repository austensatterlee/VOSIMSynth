#include "UI.h"
#include "UnitParameter.h"
#include "mutex.h"
#include "VOSIMSynth.h"

namespace syn
{
  UnitControl::UnitControl(IPlugBase* pPlug, VoiceManager* vm, Unit* unit, int x, int y, int size) :
    m_size(size),
    m_x(x),
    m_y(y),
    m_unit(unit),
    m_nParams(unit->getParameterNames().size()),
    m_is_sink(false),
    IControl(pPlug, { x,y,x + size,y + size })
  {
    for (int i = 0; i < m_nParams; i++)
    {
      m_portLabels.push_back(ITextSlider(vm,IRECT{0,0,0,0},unit->getParent().getUnitId(unit),i));
    }
    m_ports.resize(m_nParams);
    resize(size);
  }

  UnitControl::~UnitControl()
  {}

  void UnitControl::move(int newx, int newy)
  {
    if (newx < 0 || newy < 0) return;
    m_x = newx;
    m_y = newy;
    mRECT.L = newx;
    mRECT.T = newy;
    mRECT.R = newx + m_size;
    mRECT.B = newy + m_size;
    SetTargetArea(mRECT);
    resize(m_size);
  }

  void UnitControl::resize(int newsize)
  {
    if (newsize <= m_nParams * 10 + 30) newsize = m_nParams * 10 + 30;
    m_size = newsize;
    mRECT.R = m_x + m_size;
    mRECT.B = m_y + m_size;

    int portY = m_y+20;
    const vector<string>& paramNames = m_unit->getParameterNames();
    for (int i = 0; i < paramNames.size(); i++)
    {
      m_portLabels[i].setRect(IRECT{ m_x + 15 ,portY, m_x + newsize,portY + 10 });
      m_ports[i] = IRECT{ m_x, portY, m_x + 10, portY + 10 };
      portY += (newsize-20) / paramNames.size();
    }
    SetTargetArea(mRECT);
  }

  bool UnitControl::Draw(IGraphics* pGraphics)
  {
    IText textfmt{ 12,0,"Helvetica",IText::kStyleNormal,IText::kAlignNear,0,IText::kQualityClearType };
    IText textfmtwhite{ 12,&IColor{255,255,255,255},"Helvetica",IText::kStyleNormal,IText::kAlignNear,0,IText::kQualityClearType };
    IText centertextfmt{ 12,0,"Helvetica",IText::kStyleNormal,IText::kAlignCenter,0,IText::kQualityClearType };
    // If this unit is the circuit sink
    if (m_unit->getParent().getSinkId() == m_unit->getParent().getUnitId(m_unit))
    {
      pGraphics->FillIRect(&(IColor)palette[1], &mRECT);
    }
    else
    {
      pGraphics->FillIRect(&(IColor)palette[2], &mRECT);
    }

    VOSIMSynth* vs = (VOSIMSynth*)mPlug;
    // If this unit is the oscilloscope input
    if (vs->m_oscilloscope_input_unit == m_unit->getParent().getUnitId(m_unit))
    {
      pGraphics->DrawIText(&textfmt, "I", &IRECT{mRECT.R-10,mRECT.B-10,mRECT.R,mRECT.B});
    }  
    if (vs->m_oscilloscope_trigger_unit == m_unit->getParent().getUnitId(m_unit))    
    {
      pGraphics->DrawIText(&textfmt, "T", &IRECT{ mRECT.R - 20,mRECT.B - 10,mRECT.R-10,mRECT.B });
    }

    vector<string> paramNames = m_unit->getParameterNames();
    char strbuf[256];
    sprintf(strbuf,"%s",m_unit->getName().c_str());
    pGraphics->DrawIText(&centertextfmt, strbuf, &IRECT{m_x,m_y,m_x+m_size,m_y+10});
    for (int i = 0; i < paramNames.size(); i++)
    {
      pGraphics->FillCircle(&(IColor)palette[3], m_ports[i].MW(), m_ports[i].MH(), m_ports[i].W() / 2, 0, true);
      char strbuf[256];
      sprintf(strbuf, "%s", paramNames[i].c_str());
      if (!m_unit->getParam(i).isHidden())
      {
        m_portLabels[i].Draw(pGraphics);
        pGraphics->DrawIText(&textfmtwhite, strbuf, &m_portLabels[i].getRect());
      }
      else
      {
        pGraphics->DrawIText(&textfmt, strbuf, &m_portLabels[i].getRect());
      }
    }
    return true;
  }

  NDPoint<2, int> UnitControl::getPos()
  {
    return NDPoint<2, int>(m_x, m_y);
  }

  NDPoint<2, int> UnitControl::getPortPos(int port)
  {
    return NDPoint<2, int>(m_ports[port].L + m_ports[port].W() / 2, m_ports[port].T + m_ports[port].H() / 2);
  }

  NDPoint<2, int> UnitControl::getOutputPos()
  {
    return NDPoint<2, int>(m_x + m_size, m_y + m_size / 2);
  }

  Unit* UnitControl::getUnit()
  {
    return m_unit;
  }

  int UnitControl::getSelectedPort(int x, int y)
  {
    int selectedPort = -1;
    for (int i = 0; i < m_ports.size(); i++)
    {
      if (m_ports[i].Contains(x, y))
      {
        selectedPort = i;
      }
    }
    return selectedPort;
  }

  int UnitControl::getSelectedParam(int x, int y)
  {
    int selectedParam = -1;
    for (int i = 0; i < m_portLabels.size(); i++)
    {
      if (m_unit->getParam(i).isHidden())
      {
        continue;
      }
      if (m_portLabels[i].getRect().Contains(x, y))
      {
        selectedParam = i;
      }
    }
    return selectedParam;
  }

  void UnitControl::modParam(int paramid, double dX)
  {
    m_portLabels[paramid].modParam(dX);
  }

  void UnitPanel::updateInstrument()
  {
    m_vm->setMaxVoices(m_vm->getMaxVoices(), m_vm->getProtoInstrument());
  }

  void UnitPanel::deleteUnit(int unitctrlid)
  {
    WDL_MutexLock lock(&mPlug->GetGUI()->mMutex);
    // Delete unit from instrument
    Unit* unit = m_unitControls[unitctrlid]->getUnit();
    Instrument* instr = m_vm->getProtoInstrument();
    instr->removeUnit(instr->getUnitId(unit));
    // Delete unit controller
    UnitControl* unitctrl = m_unitControls[unitctrlid];
    m_unitControls.erase(unitctrlid);
    delete unitctrl;
  }

  void UnitPanel::OnMouseDown(int x, int y, IMouseMod* pMod)
  {
    if (pMod->L) m_isMouseDown = 1;
    else if (pMod->R) m_isMouseDown = 2;
    m_lastMousePos = NDPoint<2, int>(x, y);
    m_lastClickPos = m_lastMousePos;
    m_lastSelectedUnit = getSelectedUnit(x, y);
    if (m_lastSelectedUnit >= 0)
    {
      UnitControl* unitCtrl = m_unitControls[m_lastSelectedUnit];
      m_lastSelectedPort = unitCtrl->getSelectedPort(x, y);
      m_lastSelectedParam = unitCtrl->getSelectedParam(x,y);

      if (pMod->L && m_lastSelectedParam >= 0)
      {
        m_currAction = MOD_PARAM;
      }
      else if (pMod->L && m_lastSelectedPort >= 0)
      {
        m_currAction = CONNECT;
      }
      else if (pMod->C && pMod->L)
      {
        m_currAction = RESIZE;
      }
      else if (pMod->L)
      {
        m_currAction = MOVE;
      }
    }
  }

  void UnitPanel::OnMouseUp(int x, int y, IMouseMod* pMod)
  {
    WDL_MutexLock lock(&mPlug->GetGUI()->mMutex);
    Instrument* instr = m_vm->getProtoInstrument();
    int currSelectedUnit = getSelectedUnit(x, y);
    if (m_isMouseDown == 2 && currSelectedUnit == -1)
    { // Right clicking on open space
      IPopupMenu* selectedMenu = mPlug->GetGUI()->CreateIPopupMenu(&m_main_menu, x, y);
      if (selectedMenu == &m_sourceunit_menu)
      { // Create a source unit

        IPlugBase::IMutexLock lock(mPlug);
        int itemChosen = selectedMenu->GetChosenItemIdx();
        SourceUnit* srcunit = m_unitFactory->createSourceUnit(itemChosen);
        int uid = instr->addSource(srcunit);
        m_unitControls[uid] = new UnitControl(mPlug, m_vm, srcunit, x, y);
        updateInstrument();
      }
      else if (selectedMenu == &m_unit_menu)
      { // Create a non-source unit

        IPlugBase::IMutexLock lock(mPlug);
        int itemChosen = selectedMenu->GetChosenItemIdx();
        Unit* unit = m_unitFactory->createUnit(itemChosen);
        int uid = instr->addUnit(unit);
        m_unitControls[uid] = new UnitControl(mPlug, m_vm, unit, x, y);
        updateInstrument();
      }
    }
    else if (m_isMouseDown == 2 && currSelectedUnit >= 0)
    { // Right clicking on a unit

      IPopupMenu unitmenu;
      unitmenu.AddItem("Make sink");
      unitmenu.AddItem("Delete");
      unitmenu.AddItem("Set oscilloscope source");
      if (instr->isSourceUnit(currSelectedUnit))
      {
        unitmenu.AddSeparator();
        unitmenu.AddItem("Set oscilloscope trigger");
      }
      Unit* unit = m_unitControls[currSelectedUnit]->getUnit();
      IPopupMenu* selectedmenu = mPlug->GetGUI()->CreateIPopupMenu(&unitmenu, x, y);
      if (selectedmenu == &unitmenu)
      {
        IPlugBase::IMutexLock lock(mPlug);
        int selectedItem = selectedmenu->GetChosenItemIdx();
        if (selectedItem == 0)
        {
          instr->setSinkId(instr->getUnitId(unit));
          updateInstrument();
        }
        else if (selectedItem == 1)
        {
          deleteUnit(currSelectedUnit);
          updateInstrument();
        }
        else if (selectedItem == 2)
        {
          VOSIMSynth* vs = (VOSIMSynth*)mPlug;
          vs->m_oscilloscope_input_unit = instr->getUnitId(unit);
        }
        else if (selectedItem == 4)
        {
          VOSIMSynth* vs = (VOSIMSynth*)mPlug;
          vs->m_oscilloscope_trigger_unit = instr->getUnitId(unit);
        }
      }
    }
    else if (m_currAction == CONNECT && currSelectedUnit >= 0)
    {
      IPlugBase::IMutexLock lock(mPlug);
      bool result = m_vm->getProtoInstrument()->addConnection({ currSelectedUnit, m_lastSelectedUnit, m_lastSelectedPort, SET });
      updateInstrument();
    }
    m_currAction = NONE;
    m_isMouseDown = 0;
  }

  void UnitPanel::OnMouseDrag(int x, int y, int dX, int dY, IMouseMod* pMod)
  {
    NDPoint<2, int> currMousePos = NDPoint<2, int>(x, y);
    if (m_lastSelectedUnit >= 0)
    {
      UnitControl* unitCtrl = m_unitControls[m_lastSelectedUnit];
      NDPoint<2, int> unitPos = unitCtrl->getPos();
      if (m_currAction == MOD_PARAM)
      {
        unitCtrl->modParam(m_lastSelectedParam,dX);
      }
      else if (m_currAction == RESIZE)
      {
        int newsize = NDPoint<2, int>(x, y).distFrom(unitPos);
        unitCtrl->resize(newsize);
      }
      else if (m_currAction == MOVE)
      {
        NDPoint<2, int> newUnitPos = currMousePos - m_lastMousePos + unitPos;
        unitCtrl->move(newUnitPos[0], newUnitPos[1]);
      }
    }
    m_lastMousePos = currMousePos;
  }

  void UnitPanel::OnMouseDblClick(int x, int y, IMouseMod* pMod)
  {}

  bool UnitPanel::Draw(IGraphics* pGraphics)
  {
    WDL_MutexLock lock(&pGraphics->mMutex);
    pGraphics->FillIRect(&(IColor)palette[0], &mRECT);
    for (std::pair<int,UnitControl*> unitpair : m_unitControls)
    {
      unitpair.second->Draw(pGraphics);
    }
    Instrument* instr = m_vm->getProtoInstrument();
    for (std::pair<int, UnitControl*> unitpair : m_unitControls)
    {
      const vector<ConnectionMetadata>& connections = instr->getConnectionsTo(unitpair.first);
      for (int j = 0; j < connections.size(); j++)
      {
        NDPoint<2, int> pt1 = m_unitControls[connections[j].srcid]->getOutputPos();
        NDPoint<2, int> pt2 = m_unitControls[connections[j].targetid]->getPortPos(connections[j].portid);
        pGraphics->DrawLine(&IColor(255, 255, 255, 255), pt1[0], pt1[1], pt2[0], pt2[1], 0, true);
      }
    }
    if (m_currAction == CONNECT)
    {
      pGraphics->DrawLine(&IColor(255, 255, 255, 255), m_lastClickPos[0], m_lastClickPos[1], m_lastMousePos[0], m_lastMousePos[1], 0, true);
    }
    return true;
  }

  int UnitPanel::getSelectedUnit(int x, int y)
  {
    int selectedUnit = -1;
    for (std::pair<int,UnitControl*> unitpair : m_unitControls)
    {
      if (unitpair.second->IsHit(x, y))
      {
        selectedUnit = unitpair.first;
      }
    }
    return selectedUnit;
  }

}