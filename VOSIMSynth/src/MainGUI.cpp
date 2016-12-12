#include "MainGUI.h"
#include <nanogui/nanogui.h>

synui::MainGUI::MainGUI(GLFWwindow* a_window, syn::VoiceManager* a_vm, syn::UnitFactory* a_uf): m_window(a_window),
                                                                                                m_screen(nullptr),
                                                                                                m_vm(a_vm),
                                                                                                m_uf(a_uf)
{
	m_screen = new nanogui::Screen();
	m_screen->initialize(m_window, true);

	nanogui::Window* window = new nanogui::Window(m_screen, "Grid of small widgets");
	window->setPosition(Eigen::Vector2i(425, 300));
	nanogui::GridLayout* layout = new nanogui::GridLayout(nanogui::Orientation::Horizontal, 2, nanogui::Alignment::Middle, 15, 5);
	layout->setColAlignment({nanogui::Alignment::Maximum, nanogui::Alignment::Fill});
	layout->setSpacing(0, 10);
	window->setLayout(layout);

	{
		nanogui::Label* lbl = new nanogui::Label(window, "Floating point :", "sans-bold");
		lbl->setFixedSize({0,0});
		nanogui::TextBox* textBox = new nanogui::TextBox(window);
		textBox->setFixedSize(Eigen::Vector2i(0, 0));
		textBox->setEditable(true);
		textBox->setFixedSize(Eigen::Vector2i(100, 20));
		textBox->setValue("50");
		textBox->setUnits("GiB");
		textBox->setDefaultValue("0.0");
		textBox->setFontSize(16);
		textBox->setFormat("[-]?[0-9]*\\.?[0-9]+");
	}

	{
		nanogui::Label* lbl = new nanogui::Label(window, "Checkbox :", "sans-bold");
		lbl->setFixedSize({0,0});
		nanogui::CheckBox* cb = new nanogui::CheckBox(window, "Check me");
		cb->setFontSize(16);
		cb->setChecked(true);
	}


	m_screen->setVisible(true);
	m_screen->performLayout();


	// Set user pointer so we can access ourselves inside the event handlers
	glfwSetWindowUserPointer(m_window, this);


	// Setup event handlers
	glfwSetCursorPosCallback(m_window,
		[](GLFWwindow* w, double x, double y) { static_cast<synui::MainGUI*>(glfwGetWindowUserPointer(w))->m_screen->cursorPosCallbackEvent(x, y); }
	);


	glfwSetMouseButtonCallback(m_window,
		[](GLFWwindow* w, int button, int action, int modifiers) { static_cast<synui::MainGUI*>(glfwGetWindowUserPointer(w))->m_screen->mouseButtonCallbackEvent(button, action, modifiers); }
	);


	glfwSetKeyCallback(m_window,
		[](GLFWwindow* w, int key, int scancode, int action, int mods) { static_cast<synui::MainGUI*>(glfwGetWindowUserPointer(w))->m_screen->keyCallbackEvent(key, scancode, action, mods); }
	);


	glfwSetCharCallback(m_window,
		[](GLFWwindow* w, unsigned int codepoint) { static_cast<synui::MainGUI*>(glfwGetWindowUserPointer(w))->m_screen->charCallbackEvent(codepoint); }
	);


	glfwSetDropCallback(m_window,
		[](GLFWwindow* w, int count, const char** filenames) { static_cast<synui::MainGUI*>(glfwGetWindowUserPointer(w))->m_screen->dropCallbackEvent(count, filenames); }
	);


	glfwSetScrollCallback(m_window,
		[](GLFWwindow* w, double x, double y) { static_cast<synui::MainGUI*>(glfwGetWindowUserPointer(w))->m_screen->scrollCallbackEvent(x, y); }
	);


	glfwSetFramebufferSizeCallback(m_window,
		[](GLFWwindow* w, int width, int height) { static_cast<synui::MainGUI*>(glfwGetWindowUserPointer(w))->m_screen->resizeCallbackEvent(width, height); }
	);
}

void synui::MainGUI::draw()

{
	m_screen->drawContents();
	m_screen->drawWidgets();
}

synui::MainGUI::~MainGUI()
{
	delete m_screen;
	m_screen = nullptr;
}
