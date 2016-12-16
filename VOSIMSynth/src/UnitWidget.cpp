#include "UnitWidget.h"
#include "CircuitWidget.h"
#include <Unit.h>
#include <VoiceManager.h>
#include <DSPMath.h>

synui::UnitWidget::UnitWidget(synui::CircuitWidget* a_parent, int a_unitId) :
	Widget(a_parent),
	m_parentCircuit(a_parent),
	m_unitId(a_unitId)
{
	const syn::Unit& unit = m_parentCircuit->voiceManager()->getUnit(m_unitId);
	const syn::NamedContainer<syn::UnitPort, 8>& inputs = unit.inputs();
	const syn::NamedContainer<double, 8>& outputs = unit.outputs();
	
	int rowHeight = 0;
	while (rowHeight<15)
	{
		rowHeight+=m_parentCircuit->gridSize();
	}

	// Setup grid layout
	std::vector<int> rowSizes(syn::MAX(inputs.size(),outputs.size())+1, rowHeight);
	std::vector<int> colSizes{2, 0};
	auto layout = new nanogui::AdvancedGridLayout(colSizes,rowSizes);
	layout->setColStretch(0,1.0f);
	layout->setColStretch(1,1.0f);
	using Anchor = nanogui::AdvancedGridLayout::Anchor;
	setLayout(layout);
	
	// Create title
	m_titleLabel = new nanogui::Label(this, unit.name(), "sans-bold", 12);
	m_titleLabel->setEnabled(false);
	layout->setAnchor(m_titleLabel, Anchor{0,0,2,1});

	// Create port labels	
	for (int i = 0; i < inputs.size(); i++)
	{
		int inputId = inputs.indices()[i];
		const string& inputName = inputs.name(inputId);
		auto lbl = new nanogui::Label(this, inputName, "sans", 10);
		layout->setAnchor(lbl, Anchor{0,i+1});
		lbl->setId(std::to_string(inputId));
		m_inputLabels[i] = lbl;
	}
	for (int i = 0; i < outputs.size(); i++)
	{
		int outputId = outputs.indices()[i];
		const string& outputName = outputs.name(outputId);
		auto lbl = new nanogui::Label(this, outputName, "sans", 10);
		lbl->setTextAlign(nanogui::Label::Alignment::Right);
		layout->setAnchor(lbl, Anchor{1,i+1});
		lbl->setId(std::to_string(outputId));
		m_outputLabels[i] = lbl;
	}
}

void synui::UnitWidget::draw(NVGcontext* ctx)
{
	nvgSave(ctx);
	nvgTranslate(ctx, mPos.x(), mPos.y());
	nvgFillColor(ctx, nanogui::Color(0.3f, 0.75f));
    nvgBeginPath(ctx);
	nvgRect(ctx, 0, 0, mSize.x(), mSize.y());
	nvgFill(ctx);
	nvgRestore(ctx);
	Widget::draw(ctx);
}

bool synui::UnitWidget::mouseButtonEvent(const Eigen::Vector2i& p, int button, bool down, int modifiers)
{	
	Eigen::Vector2i mousePos = p - position();
	if(button==GLFW_MOUSE_BUTTON_LEFT && down){
		// Check if title was clicked
		if(m_titleLabel->contains(mousePos)){
			m_drag = true;
			return true;
		}

		// Check if an input port was clicked
		for(auto inputLabel : m_inputLabels){
			if(inputLabel.second->contains(mousePos))
			{
				m_parentCircuit->startWireDraw_(m_unitId, inputLabel.first, false);
				return true;
			}
		}

		// Check if an output port was clicked
		for(auto outputLabel : m_outputLabels){
			if(outputLabel.second->contains(mousePos))
			{
				m_parentCircuit->startWireDraw_(m_unitId, outputLabel.first, true);
				return true;
			}
		}
	}else if(button==GLFW_MOUSE_BUTTON_LEFT && !down)
	{
		m_drag = false;

		// Check if mouse was release over an input port
		for(auto inputLabel : m_inputLabels){
			if(inputLabel.second->contains(mousePos))
			{
				m_parentCircuit->endWireDraw_(m_unitId, inputLabel.first, false);
				return true;
			}
		}

		// Check if mouse was release over an output port
		for(auto outputLabel : m_outputLabels){
			if(outputLabel.second->contains(mousePos))
			{
				m_parentCircuit->endWireDraw_(m_unitId, outputLabel.first, true);
				return true;
			}
		}		
	}
	return false;
}
bool synui::UnitWidget::mouseDragEvent(const Eigen::Vector2i& p, const Eigen::Vector2i& rel, int button, int modifiers)
{ 
	if(m_drag){
		mPos = p;
		mPos = mPos.cwiseMax(Eigen::Vector2i::Zero());
		mPos = mPos.cwiseMin(parent()->size() - mSize);	
		mPos = m_parentCircuit->fixToGrid(mPos);
		return true;
	}
	return false;
}

Eigen::Vector2i synui::UnitWidget::getInputPortPosition(int a_portId)
{
	auto portPos = m_inputLabels[a_portId]->position();
	portPos.y() += m_inputLabels[a_portId]->size().y()*0.5;
	return portPos + position();
}
Eigen::Vector2i synui::UnitWidget::getOutputPortPosition(int a_portId)
{	
	auto portPos = m_outputLabels[a_portId]->position();
	portPos.x() += m_outputLabels[a_portId]->size().x();
	portPos.y() += m_outputLabels[a_portId]->size().y()*0.5;
	return portPos + position();
}
