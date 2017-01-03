#include "MainGUI.h"
#include "UnitFactory.h"
#include <nanogui/nanogui.h>
#include <nanogui/theme.h>
#include "MainWindow.h"
#include "VoiceManager.h"
#include "CircuitWidget.h"
#include "UnitEditor.h"

namespace synui
{
    class EnhancedWindow : public nanogui::Window
    {
    public:
        typedef std::function<void(EnhancedWindow*, NVGcontext*)> DrawFunc;

        EnhancedWindow(Widget* a_parent, const std::string& a_title)
            : Window(a_parent, a_title)
        {
            if (!a_title.empty())
            {
                // Close button
                auto settings_close_button = new nanogui::Button(buttonPanel(), "", ENTYPO_CROSS);
                settings_close_button->setFixedWidth(18);
                settings_close_button->setCallback([this]() { setVisible(false); });
            }
        }

        bool mouseButtonEvent(const Vector2i& p, int button, bool down, int modifiers) override
        {
            if (!title().empty())
            {
                return Window::mouseButtonEvent(p, button, down, modifiers);
            }
            else
            {
                if (Widget::mouseButtonEvent(p, button, down, modifiers))
                    return true;
                return false;
            }
        }

        void draw(NVGcontext* ctx) override
        {
            if (m_drawCallback)
            {
                nvgSave(ctx);
                m_drawCallback(this, ctx);
                nvgRestore(ctx);
                Widget::draw(ctx);
            }
            else
            {
                Window::draw(ctx);
            }
        }

        nanogui::Button* createOpenButton(Widget* a_parent, const string& text = "", int icon = 0, std::function<void()> a_callback = nullptr)
        {
            auto settings_button = new nanogui::Button(a_parent, text, icon);
            settings_button->setFixedSize({20,20});
            settings_button->setCallback([this, a_callback]()
                {
                    setVisible(true);
                    screen()->moveWindowToFront(this);
                    if (a_callback)
                        a_callback();
                });
            return settings_button;
        }

        void setDrawCallback(DrawFunc f) { m_drawCallback = f; }
        DrawFunc getDrawCallback() const { return m_drawCallback; }

    private:
        DrawFunc m_drawCallback;
    };
}

synui::MainGUI::operator json() const
{
    json j;
    j["circuit"] = m_circuit->operator json();
    j["settings"] = json();

    return j;
}

synui::MainGUI* synui::MainGUI::load(const json& j)
{
    m_circuit->load(j["circuit"]);
    return this;
}

void synui::MainGUI::reset()
{
    m_unitEditorHost->reset();
    m_circuit->reset();
}

void synui::MainGUI::_createUnitSelector(nanogui::Widget* a_widget)
{
    nanogui::BoxLayout* layout = new nanogui::BoxLayout(nanogui::Orientation::Vertical, nanogui::Alignment::Fill, 1, 1);
    a_widget->setLayout(layout);

    auto unitSelectorTitle = new nanogui::Widget(a_widget);
    unitSelectorTitle->setLayout(new nanogui::BoxLayout(nanogui::Orientation::Horizontal, nanogui::Alignment::Middle, 2, 10));

    for (auto gname : m_uf->getGroupNames())
    {
        nanogui::PopupButton* button = new nanogui::PopupButton(a_widget, gname);
        button->setDisposable(true);
        button->setFontSize(15);
        nanogui::Popup* popup = button->popup();
        popup->setLayout(new nanogui::BoxLayout(nanogui::Orientation::Vertical, nanogui::Alignment::Fill, 0, 0));
        for (auto uname : m_uf->getPrototypeNames(gname))
        {
            nanogui::Button* subbtn = new nanogui::Button(popup, uname);
            subbtn->setId(uname);
            subbtn->setFontSize(14);
            // Close popup
            subbtn->setCallback([this, button, subbtn]()
                {
                    this->m_circuit->loadPrototype(subbtn->id());
                    button->setPushed(false);
                    m_screen->updateFocus(nullptr);
                });
        }
    }
}

void synui::MainGUI::_createSettingsEditor(nanogui::Widget* a_widget, nanogui::FormHelper* a_fh)
{
    auto layout = new nanogui::AdvancedGridLayout({10, 0, 10, 0}, {});
    layout->setMargin(10);
    layout->setColStretch(2, 1);
    a_widget->setLayout(layout);
    nanogui::FormHelper* helper = a_fh;
    helper->setWidget(a_widget);

    helper->addVariable<int>("Grid Spacing", [this](const int& s)
                             {
                                 m_circuit->resizeGrid(s);
                                 m_screen->performLayout();
                             }, [this]()
                             {
                                 int gs = m_circuit->getGridSpacing();
                                 return gs;
                             });

    helper->addVariable<int>("Window width", [this](const int& w)
    {
        m_window->resize(w, m_screen->height());
    }, [this]()
    {
        return m_screen->width();
    });

    helper->addVariable<int>("Window height", [this](const int& h)
    {
        m_window->resize(m_screen->width(), h);
    }, [this]()
    {
        return m_screen->height();
    });

    nanogui::Theme* theme = m_screen->theme();

    /* Spacing-related parameters */
    helper->addGroup("Spacing");
    helper->addVariable("Standard Font Size", theme->mStandardFontSize);
    helper->addVariable("Button Font Size", theme->mButtonFontSize);
    helper->addVariable("Text Box Font Size", theme->mTextBoxFontSize);
    helper->addVariable("Window Corner Radius", theme->mWindowCornerRadius);
    helper->addVariable("Window Drop Shadow Size", theme->mWindowDropShadowSize);
    helper->addVariable("Button Corner Radius", theme->mButtonCornerRadius);

    /* Generic colors */
    helper->addGroup("Global Colors");
    helper->addVariable("Border Dark", theme->mBorderDark);
    helper->addVariable("Border Light", theme->mBorderLight);
    helper->addVariable("Border Medium", theme->mBorderMedium);
    helper->addVariable("Drop Shadow", theme->mDropShadow);
    helper->addVariable("Icon Color", theme->mIconColor);
    helper->addVariable("Transparent", theme->mTransparent);
    helper->addVariable("Text Color", theme->mTextColor);
    helper->addVariable("Text Color Shadow", theme->mTextColorShadow);
    helper->addVariable("Disabled Text Color", theme->mDisabledTextColor);

    /* Button colors */
    helper->addGroup("Button Colors");
    helper->addVariable("Button Top (Focused)", theme->mButtonGradientTopFocused);
    helper->addVariable("Button Top (Unfocused)", theme->mButtonGradientTopUnfocused);
    helper->addVariable("Button Top (Pushed)", theme->mButtonGradientTopPushed);
    helper->addVariable("Button Bot (Focused)", theme->mButtonGradientBotFocused);
    helper->addVariable("Button Bot (Unfocused)", theme->mButtonGradientBotUnfocused);
    helper->addVariable("Button Bot (Pushed)", theme->mButtonGradientBotPushed);

    /* Window colors */
    helper->addGroup("Window Colors");
    helper->addVariable("Window Fill (Unfocused)", theme->mWindowFillUnfocused);
    helper->addVariable("Window Fill (Focused)", theme->mWindowFillFocused);
    helper->addVariable("Window Title (Unfocused)", theme->mWindowTitleUnfocused);
    helper->addVariable("Window Title (Focused)", theme->mWindowTitleFocused);

    helper->addVariable("Window Header Top", theme->mWindowHeaderGradientTop);
    helper->addVariable("Window Header Bot", theme->mWindowHeaderGradientBot);
    helper->addVariable("Window Header Sep Top", theme->mWindowHeaderSepTop);
    helper->addVariable("Window Header Sep Bot", theme->mWindowHeaderSepBot);

    helper->addVariable("Window Popup", theme->mWindowPopup);
    helper->addVariable("Window Popup Transparent", theme->mWindowPopupTransparent);
}

void synui::MainGUI::_createLogViewer(nanogui::Widget* a_widget)
{
    auto layout = new nanogui::BoxLayout(nanogui::Orientation::Vertical, nanogui::Alignment::Fill, 10, 5);
    a_widget->setLayout(layout);

    Logger::instance().addListener([a_widget]()
        {
            const auto& log = Logger::instance().getLogs().back();
            std::ostringstream value;
            value << "[ " << log.first << "] " << log.second;
            auto txt = new nanogui::Label(a_widget, value.str(), "sans", 10);
            a_widget->screen()->performLayout();
        });
}

synui::MainGUI::MainGUI(synui::MainWindow* a_window, syn::VoiceManager* a_vm, syn::UnitFactory* a_uf) : m_window(a_window),
                                                                                                        m_screen(nullptr),
                                                                                                        m_vm(a_vm),
                                                                                                        m_uf(a_uf)
{
    m_screen = new nanogui::Screen();
    m_screen->initialize(m_window->getWindow(), true);

    m_screen->theme()->mWindowDropShadowSize = 2;

    /* Divide window into panes */
    m_sidePanelL = new synui::EnhancedWindow(m_screen, "");
    m_sidePanelL->setIsBackgroundWindow(true);
    m_sidePanelL->setLayout(new nanogui::BoxLayout(nanogui::Orientation::Vertical, nanogui::Alignment::Fill, 5, 1));
    m_sidePanelL->setFixedHeight(m_screen->height());
    m_sidePanelL->setFixedWidth(200);
    m_screen->performLayout();

    m_sidePanelR = new synui::EnhancedWindow(m_screen, "");
    m_sidePanelR->setIsBackgroundWindow(true);
    m_sidePanelR->setLayout(new nanogui::BoxLayout(nanogui::Orientation::Vertical, nanogui::Alignment::Fill, 0, 0));
    m_screen->performLayout();

    m_tabWidget = new nanogui::TabWidget(m_sidePanelL);

    /* Create unit selector tab. */
    m_unitSelector = m_tabWidget->createTab("Build");
    _createUnitSelector(m_unitSelector);

    /* Create unit editor tab. */
    m_unitEditorHost = new UnitEditorHost(nullptr, m_vm);
    m_tabWidget->addTab("Editor", m_unitEditorHost);
    m_unitEditorHost->setLayout(new nanogui::GridLayout(nanogui::Orientation::Horizontal, 1, nanogui::Alignment::Fill, 1));

    m_circuit = new CircuitWidget(m_sidePanelR, m_window, m_unitEditorHost, a_vm, a_uf);

    /* Create button panel */
    m_buttonPanel = new EnhancedWindow(m_screen, "");
    m_buttonPanel->setIsBackgroundWindow(true);
    m_buttonPanel->setDrawCallback([](EnhancedWindow* self, NVGcontext* ctx)
        {
            nvgBeginPath(ctx);
            nvgRect(ctx, self->position().x(), self->position().y(), self->width(), self->height());

            nanogui::Color fillColor = self->theme()->mWindowFillUnfocused * 0.5f;
            nvgFillColor(ctx, fillColor);
            nvgFill(ctx);
        });
    auto buttonPanelLayout = new nanogui::AdvancedGridLayout({}, {0}, 5);
    m_buttonPanel->setLayout(buttonPanelLayout);
    buttonPanelLayout->appendCol(0, 1.0);

    /* Create log viewer window. */
    m_logViewer = new synui::EnhancedWindow(m_screen, "Logs");
    m_logViewer->setVisible(false);
    auto logScrollPanel = new nanogui::VScrollPanel(m_logViewer);
    m_logViewer->setLayout(new nanogui::BoxLayout(nanogui::Orientation::Vertical, nanogui::Alignment::Fill));
    logScrollPanel->setFixedHeight(400);
    _createLogViewer(new nanogui::Widget(logScrollPanel));

    /* Add a button for openning the log viewer window. */
    buttonPanelLayout->appendCol(0, 0);
    
    auto log_callback = [this]()
    {
        m_screen->centerWindow(m_logViewer);
    };
    auto logviewer_button = m_logViewer->createOpenButton(m_buttonPanel, "", ENTYPO_TRAFFIC_CONE, log_callback);
    buttonPanelLayout->setAnchor(logviewer_button, nanogui::AdvancedGridLayout::Anchor{buttonPanelLayout->colCount() - 1,0});

    /* Create the settings editor window. */
    m_settingsEditor = new EnhancedWindow(m_screen, "Settings");
    m_settingsEditor->setVisible(false);
    auto settingsScrollPanel = new nanogui::VScrollPanel(m_settingsEditor);
    settingsScrollPanel->setFixedHeight(400);
    m_settingsEditor->setLayout(new nanogui::BoxLayout(nanogui::Orientation::Vertical, nanogui::Alignment::Fill));
    m_settingsFormHelper = std::make_shared<nanogui::FormHelper>(m_screen);
    _createSettingsEditor(new nanogui::Widget(settingsScrollPanel), m_settingsFormHelper.get());

    /* Add a button for openning the settings editor window. */
    buttonPanelLayout->appendCol(0, 0);
    auto settings_callback = [this]()
            {
                m_settingsFormHelper->refresh();
                m_screen->centerWindow(m_settingsEditor);
            };
    auto settings_button = m_settingsEditor->createOpenButton(m_buttonPanel, "", ENTYPO_COG, settings_callback);
    buttonPanelLayout->setAnchor(settings_button, nanogui::AdvancedGridLayout::Anchor{buttonPanelLayout->colCount() - 1,0});

    m_tabWidget->setActiveTab(0);

    /* Setup event handlers. */

    glfwSetWindowUserPointer(m_window->getWindow(), this);// Set user pointer so we can access ourselves inside the event handlers

    glfwSetCursorPosCallback(m_window->getWindow(),
                             [](GLFWwindow* w, double x, double y) { static_cast<synui::MainGUI*>(glfwGetWindowUserPointer(w))->m_screen->cursorPosCallbackEvent(x, y); }
    );

    glfwSetMouseButtonCallback(m_window->getWindow(),
                               [](GLFWwindow* w, int button, int action, int modifiers) { static_cast<synui::MainGUI*>(glfwGetWindowUserPointer(w))->m_screen->mouseButtonCallbackEvent(button, action, modifiers); }
    );

    glfwSetKeyCallback(m_window->getWindow(),
                       [](GLFWwindow* w, int key, int scancode, int action, int mods) { static_cast<synui::MainGUI*>(glfwGetWindowUserPointer(w))->m_screen->keyCallbackEvent(key, scancode, action, mods); }
    );

    glfwSetCharCallback(m_window->getWindow(),
                        [](GLFWwindow* w, unsigned int codepoint) { static_cast<synui::MainGUI*>(glfwGetWindowUserPointer(w))->m_screen->charCallbackEvent(codepoint); }
    );
    
    glfwSetDropCallback(m_window->getWindow(),
                        [](GLFWwindow* w, int count, const char** filenames) { static_cast<synui::MainGUI*>(glfwGetWindowUserPointer(w))->m_screen->dropCallbackEvent(count, filenames); }
    );

    glfwSetScrollCallback(m_window->getWindow(),
                          [](GLFWwindow* w, double x, double y) { static_cast<synui::MainGUI*>(glfwGetWindowUserPointer(w))->m_screen->scrollCallbackEvent(x, y); }
    );

    auto resizeCallback = [](GLFWwindow* w, int width, int height)
            {
                auto mainGui = static_cast<synui::MainGUI*>(glfwGetWindowUserPointer(w));
                mainGui->m_screen->resizeCallbackEvent(width, height);
                mainGui->m_sidePanelL->setFixedHeight(mainGui->m_screen->height());
                mainGui->m_circuit->setFixedHeight(mainGui->m_screen->height());
                mainGui->m_circuit->setFixedWidth(mainGui->m_screen->width() - mainGui->m_sidePanelL->width());
                mainGui->m_circuit->resizeGrid(mainGui->m_circuit->getGridSpacing());
                mainGui->m_sidePanelR->setPosition({mainGui->m_sidePanelL->width(), mainGui->m_buttonPanel->height()});
                mainGui->m_buttonPanel->setPosition({mainGui->m_sidePanelL->width(), 0});
                mainGui->m_buttonPanel->setFixedWidth(mainGui->m_sidePanelR->width());
                mainGui->m_screen->performLayout();
            };
    glfwSetFramebufferSizeCallback(m_window->getWindow(), resizeCallback);
    glfwSetWindowSizeCallback(m_window->getWindow(), resizeCallback);
}

void synui::MainGUI::show()
{
    m_screen->setVisible(true);
    m_screen->performLayout();
}

void synui::MainGUI::hide() { m_screen->setVisible(false); }

void synui::MainGUI::draw() { m_screen->drawAll(); }

synui::MainGUI::~MainGUI() {}
