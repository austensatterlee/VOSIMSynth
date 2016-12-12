#include "MainGUI.h"
#include "MainWindow.h"
#include "vosimsynth_resources.h"
#include <VoiceManager.h>
#include <UnitFactory.h>
#include <SFML/System/Utf.hpp>

sf::String utf8to32(std::string a_src)
{
	std::basic_string<sf::Uint32> tmp;
	sf::Utf<8>::toUtf32( a_src.begin(), a_src.end(), std::back_inserter( tmp ) );
	return tmp;
}

tgui::MenuBar::Ptr synui::MainGUI::createUnitSelector()
{
	tgui::MenuBar::Ptr group_selector = m_theme->load("MenuBar");
	
	auto unit_selector = m_uf->getGroupNames();
	vector<tgui::ListBox::Ptr> unit_selectors;
	for (const string& group_name : unit_selector) {
		group_selector->addMenu(group_name);
		auto unit_names = m_uf->getPrototypeNames(group_name);
		for (const string& unit_name : unit_names) {
			group_selector->addMenuItem(group_name, unit_name);
		}
	}

	group_selector->connect("MenuItemClicked", [this](const std::vector<sf::String> a_selection)
	{
		std::string group_name = a_selection[0];
		std::string unit_name = a_selection[1];
		m_unitToCreate = std::make_pair(group_name, unit_name);
	});

	return group_selector;
}

tgui::Panel::Ptr synui::MainGUI::createCircuitController() {return m_theme->load("Panel");}
tgui::Panel::Ptr synui::MainGUI::createUnitEditor() {return m_theme->load("Panel");}

synui::MainGUI::MainGUI(MainWindow& a_window, syn::VoiceManager* a_vm, syn::UnitFactory* a_uf) :
	m_window(a_window),
	m_gui(*a_window.getWindow()),
	m_theme(nullptr),
	m_vm(a_vm),
	m_uf(a_uf)
{	
	// Load resources
	m_theme = std::make_shared<tgui::Theme>("widgets/TransparentGrey.txt");
	m_fonts["entypo"].loadFromMemory(entypo_ttf, entypo_ttf_size);
	m_fonts["roboto_regular"].loadFromMemory(roboto_regular_ttf, roboto_regular_ttf_size);
	m_gui.setFont(m_fonts["roboto_regular"]);

	// Create unit selection menu
	auto us = createUnitSelector();
	us->setPosition(0, 0);
	m_gui.add(us, "unit_selector");


	//m_gui.add(createCircuitController(), "circuit_controller");
	//m_gui.add(createUnitEditor(), "unit editor");
}

void synui::MainGUI::draw() { m_gui.draw(); }
