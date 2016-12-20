#include "MainGUI.h"
#include "UnitFactory.h"
#include <nanogui/nanogui.h>
#include <nanogui/theme.h>
#include "MainWindow.h"
#include "Circuit.h"
#include "VoiceManager.h"
#include "CircuitWidget.h"

namespace synui
{
    class StaticWindow : public nanogui::Window
    {
    public:
        StaticWindow(Widget *parent, const std::string &title)
            : Window(parent, title) {}

        bool mouseButtonEvent(const Eigen::Vector2i &p, int button, bool down, int modifiers) override
        {
            if (Widget::mouseButtonEvent(p, button, down, modifiers))
                return true;
            return false;
        }
    };
}

nanogui::Widget *synui::MainGUI::_createUnitSelector(nanogui::Widget *a_widget) const
{
    nanogui::BoxLayout *layout = new nanogui::BoxLayout(nanogui::Orientation::Vertical, nanogui::Alignment::Fill, 1, 1);
    a_widget->setLayout(layout);

    auto unitSelectorTitle = new nanogui::Widget(a_widget);
    unitSelectorTitle->setLayout(new nanogui::BoxLayout(nanogui::Orientation::Horizontal, nanogui::Alignment::Middle, 2, 10));

    for (auto gname : m_uf->getGroupNames())
    {
        nanogui::PopupButton *button = new nanogui::PopupButton(a_widget, gname);
        nanogui::Popup *popup = button->popup();

        popup->setLayout(new nanogui::BoxLayout(nanogui::Orientation::Vertical, nanogui::Alignment::Fill, 0, 0));
        for (auto uname : m_uf->getPrototypeNames(gname))
        {
            nanogui::Button *subbtn = new nanogui::Button(popup, uname);
            subbtn->setId(uname);
            subbtn->setFontSize(18);
            // Close popup
            subbtn->setCallback([this, button, subbtn]()
                {
                    this->m_circuit->loadPrototype(subbtn->id());
                    button->popup()->setVisible(false);
                    this->m_tabWidget->setActiveTab(1);
                    //button->mouseButtonEvent(button->position() + button->size() / 2, GLFW_MOUSE_BUTTON_LEFT, true, 0);
                });
        }
    }
    return a_widget;
}

synui::MainGUI::MainGUI(synui::MainWindow *a_window, syn::VoiceManager *a_vm, syn::UnitFactory *a_uf) : m_window(a_window),
                                                                                                        m_screen(nullptr),
                                                                                                        m_vm(a_vm),
                                                                                                        m_uf(a_uf)
{
    m_screen = new nanogui::Screen();
    m_screen->initialize(m_window->getWindow(), true);

    ////
    // Divide window into panes
    //
    m_sidePanelL = new synui::StaticWindow(m_screen, "");
    m_sidePanelL->setLayout(new nanogui::BoxLayout(nanogui::Orientation::Vertical, nanogui::Alignment::Fill, 5, 1));
    m_sidePanelL->setFixedHeight(m_screen->height());
    m_sidePanelL->setFixedWidth(200);
    m_screen->performLayout();

    m_sidePanelR = new synui::StaticWindow(m_screen, "");
    m_sidePanelR->setLayout(new nanogui::BoxLayout(nanogui::Orientation::Vertical, nanogui::Alignment::Fill, 5, 1));
    m_screen->performLayout();

    m_tabWidget = new nanogui::TabWidget(m_sidePanelL);
    ////
    // Create unit selector tab.
    m_unitSelector = _createUnitSelector(m_tabWidget->createTab("Build"));
    ////
    // Create unit editor tab.
    m_unitEditorHost = m_tabWidget->createTab("Editor"); 
    m_unitEditorHost->setLayout(new nanogui::BoxLayout(nanogui::Orientation::Vertical, nanogui::Alignment::Fill, 1, 1));

    m_tabWidget->setActiveTab(0);

    m_circuit = new CircuitWidget(m_sidePanelR, m_window, m_unitEditorHost, a_vm, a_uf);


    ////
    // Setup event handlers.
    //
    // Set user pointer so we can access ourselves inside the event handlers
    glfwSetWindowUserPointer(m_window->getWindow(), this);

    glfwSetCursorPosCallback(m_window->getWindow(),
        [](GLFWwindow *w, double x, double y) { static_cast<synui::MainGUI*>(glfwGetWindowUserPointer(w))->m_screen->cursorPosCallbackEvent(x, y); }
    );


    glfwSetMouseButtonCallback(m_window->getWindow(),
        [](GLFWwindow *w, int button, int action, int modifiers) { static_cast<synui::MainGUI*>(glfwGetWindowUserPointer(w))->m_screen->mouseButtonCallbackEvent(button, action, modifiers); }
    );


    glfwSetKeyCallback(m_window->getWindow(),
        [](GLFWwindow *w, int key, int scancode, int action, int mods) { static_cast<synui::MainGUI*>(glfwGetWindowUserPointer(w))->m_screen->keyCallbackEvent(key, scancode, action, mods); }
    );


    glfwSetCharCallback(m_window->getWindow(),
        [](GLFWwindow *w, unsigned int codepoint) { static_cast<synui::MainGUI*>(glfwGetWindowUserPointer(w))->m_screen->charCallbackEvent(codepoint); }
    );


    glfwSetDropCallback(m_window->getWindow(),
        [](GLFWwindow *w, int count, const char** filenames) { static_cast<synui::MainGUI*>(glfwGetWindowUserPointer(w))->m_screen->dropCallbackEvent(count, filenames); }
    );


    glfwSetScrollCallback(m_window->getWindow(),
        [](GLFWwindow *w, double x, double y) { static_cast<synui::MainGUI*>(glfwGetWindowUserPointer(w))->m_screen->scrollCallbackEvent(x, y); }
    );

    auto resizeCallback = [](GLFWwindow *w, int width, int height)
            {
                auto mainGui = static_cast<synui::MainGUI*>(glfwGetWindowUserPointer(w));
                mainGui->m_screen->resizeCallbackEvent(width, height);
                mainGui->m_sidePanelL->setFixedHeight(mainGui->m_screen->height());
                mainGui->m_circuit->setFixedHeight(mainGui->m_screen->height() - 10);
                mainGui->m_circuit->setFixedWidth(mainGui->m_screen->width() - mainGui->m_sidePanelL->width() - 10);
                mainGui->m_sidePanelR->setPosition({mainGui->m_sidePanelL->width(), 0});
                mainGui->m_screen->performLayout();
            };
    glfwSetFramebufferSizeCallback(m_window->getWindow(), resizeCallback);
    glfwSetWindowSizeCallback(m_window->getWindow(), resizeCallback);
}

void synui::MainGUI::show() const
{
    m_screen->setVisible(true);
    m_screen->performLayout();
}

void synui::MainGUI::hide() const { m_screen->setVisible(false); }

void synui::MainGUI::draw() { m_screen->drawAll(); }

synui::MainGUI::~MainGUI()
{
    delete m_screen;
    m_screen = nullptr;
}
