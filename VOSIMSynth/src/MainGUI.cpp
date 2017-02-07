#include "MainGUI.h"
#include "UnitFactory.h"
#include <nanogui/nanogui.h>
#include <nanogui/theme.h>
#include "MainWindow.h"
#include "VoiceManager.h"
#include "CircuitWidget.h"
#include "UnitEditor.h"
#include "Logger.h"

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
                m_drawCallback(this, ctx);
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
    json& settings = j["settings"] = json();
    settings = m_settingsFormHelper->operator json();
    
    return j;
}

synui::MainGUI* synui::MainGUI::load(const json& j)
{
    reset();
    const json& settings = j.value("settings", json());
    if (!settings.empty())
    {
        m_settingsFormHelper->load(settings);
    }
    m_circuit->load(j["circuit"]);
    return this;
}

void synui::MainGUI::reset()
{
    m_unitEditorHost->reset();
    m_circuit->reset();
}

void synui::MainGUI::resize(int a_w, int a_h)
{
    m_screen->resizeCallbackEvent(a_w, a_h);
    m_sidePanelL->setFixedHeight(m_screen->height());
    m_sidePanelR->setPosition({m_sidePanelL->width(), m_buttonPanel->height()});
    m_sidePanelR->setFixedWidth(m_screen->width() - m_sidePanelR->absolutePosition().x());
    m_circuit->setFixedHeight(m_screen->height() - m_sidePanelR->absolutePosition().y());
    m_circuit->setFixedWidth(m_screen->width() - m_sidePanelR->absolutePosition().x());
    m_circuit->resizeGrid(m_circuit->getGridSpacing());
    m_buttonPanel->setPosition({m_sidePanelL->width(), 0});
    m_buttonPanel->setFixedWidth(m_sidePanelR->width());
    m_screen->performLayout();
}

void synui::MainGUI::initialize_(GLFWwindow* a_window)
{
    /* Setup event handlers. */
    glfwSetWindowUserPointer(a_window, this);// Set user pointer so we can access ourselves inside the event handlers

    glfwSetCursorPosCallback(a_window,
        [](GLFWwindow* w, double x, double y) { static_cast<synui::MainGUI*>(glfwGetWindowUserPointer(w))->m_screen->cursorPosCallbackEvent(x, y); }
    );

    glfwSetMouseButtonCallback(a_window,
        [](GLFWwindow* w, int button, int action, int modifiers) { static_cast<synui::MainGUI*>(glfwGetWindowUserPointer(w))->m_screen->mouseButtonCallbackEvent(button, action, modifiers); }
    );

    glfwSetKeyCallback(a_window,
        [](GLFWwindow* w, int key, int scancode, int action, int mods) { static_cast<synui::MainGUI*>(glfwGetWindowUserPointer(w))->m_screen->keyCallbackEvent(key, scancode, action, mods); }
    );

    glfwSetCharCallback(a_window,
        [](GLFWwindow* w, unsigned int codepoint) { static_cast<synui::MainGUI*>(glfwGetWindowUserPointer(w))->m_screen->charCallbackEvent(codepoint); }
    );

    glfwSetDropCallback(a_window,
        [](GLFWwindow* w, int count, const char** filenames) { static_cast<synui::MainGUI*>(glfwGetWindowUserPointer(w))->m_screen->dropCallbackEvent(count, filenames); }
    );

    glfwSetScrollCallback(a_window,
        [](GLFWwindow* w, double x, double y) { static_cast<synui::MainGUI*>(glfwGetWindowUserPointer(w))->m_screen->scrollCallbackEvent(x, y); }
    );

    auto resizeCallback = [](GLFWwindow* w, int width, int height)
            {
                auto mainGui = static_cast<synui::MainGUI*>(glfwGetWindowUserPointer(w));
                mainGui->resize(width, height);
            };
    glfwSetFramebufferSizeCallback(a_window, resizeCallback);
    glfwSetWindowSizeCallback(a_window, resizeCallback);
    m_screen->initialize(a_window, true);
}

void synui::MainGUI::createUnitSelector_(nanogui::Widget* a_widget)
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
        button->setCallback([this, popup]() { m_screen->moveWindowToFront(popup); });
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

void synui::MainGUI::createSettingsEditor_(nanogui::Widget* a_widget, SerializableFormHelper* a_fh)
{
    auto layout = new nanogui::AdvancedGridLayout({10, 0, 10, 0}, {});
    layout->setMargin(10);
    layout->setColStretch(2, 1);
    a_widget->setLayout(layout);
    SerializableFormHelper* helper = a_fh;
    helper->setWidget(a_widget);

    helper->addGroup("Plugin Settings");

    helper->addSerializableVariable<int>("Grid Spacing", [this](const int& s)
        {
            m_circuit->resizeGrid(s);
            m_screen->performLayout();
        }, [this]()
        {
            int gs = m_circuit->getGridSpacing();
            return gs;
        });

    helper->addSerializableVariable<int>("Window width", [this](const int& w) { m_window->resize(w, m_screen->height()); }, [this]() { return m_screen->width(); });
    helper->addSerializableVariable<int>("Window height", [this](const int& h) { m_window->resize(m_screen->width(), h); }, [this]() { return m_screen->height(); });
    helper->addSerializableVariable<int>("Internal buf. size",
        [this, helper](const int& size)
        {
            syn::RTMessage* msg = new syn::RTMessage;
            PutArgs(&msg->data, m_vm, size, helper);
            msg->action = [](syn::Circuit*, bool a_isLast, ByteChunk* a_data)
            {
                if(a_isLast){
                    syn::VoiceManager* vm;
                    int internalBufferSize;
                    nanogui::FormHelper* helper;
                    GetArgs(a_data, 0, vm, internalBufferSize, helper);
                    vm->setInternalBufferSize(internalBufferSize);
                    helper->refresh();
                }
            };
            m_vm->queueAction(msg);
        }, [this]()
        {
            return m_vm->getInternalBufferSize();
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

void synui::MainGUI::createLogViewer_(nanogui::Widget* a_widget)
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
    initialize_(m_window->getWindow());

    m_screen->theme()->mWindowDropShadowSize = 2;

    /* Create left pane. */
    m_sidePanelL = new synui::EnhancedWindow(m_screen, "");
    m_sidePanelL->setIsBackgroundWindow(true);
    m_sidePanelL->setLayout(new nanogui::GridLayout(nanogui::Orientation::Horizontal, 1, nanogui::Alignment::Fill, 5, 0));
    m_sidePanelL->setFixedHeight(m_screen->height());
    m_sidePanelL->setFixedWidth(200);
    m_sidePanelL->setDrawCallback([](EnhancedWindow* self, NVGcontext* ctx)
        {
            nanogui::Color fillColor = self->theme()->mWindowFillUnfocused.cwiseProduct(nanogui::Color(0.9f, 1.0f));

            nvgSave(ctx);

            nvgTranslate(ctx, self->position().x(), self->position().y());
            nvgBeginPath(ctx);
            nvgRoundedRect(ctx, 0, 0, self->width(), self->height(), 1.0f);
            nvgFillColor(ctx, fillColor);
            nvgFill(ctx);

            nvgRestore(ctx);

            self->Widget::draw(ctx);
        });

    /* Create tab widget in left pane. */
    m_tabWidget = new nanogui::TabWidget(m_sidePanelL);

    /* Create unit selector tab. */
    m_unitSelector = m_tabWidget->createTab("Build");
    createUnitSelector_(m_unitSelector);

    /* Create unit editor tab. */
    m_unitEditorHost = new UnitEditorHost(nullptr, m_vm);
    m_tabWidget->addTab("Editor", m_unitEditorHost);
    m_unitEditorHost->setLayout(new nanogui::GridLayout(nanogui::Orientation::Horizontal, 1, nanogui::Alignment::Fill, 1));

    m_tabWidget->setActiveTab(0);

    /* Create right pane. */
    m_sidePanelR = new synui::EnhancedWindow(m_screen, "");
    m_sidePanelR->setIsBackgroundWindow(true);
    m_sidePanelR->setLayout(new nanogui::BoxLayout(nanogui::Orientation::Vertical, nanogui::Alignment::Fill, 0, 0));
    m_sidePanelR->setDrawCallback([](EnhancedWindow* self, NVGcontext* ctx)
        {
            nanogui::Color fillColor = self->theme()->mWindowFillUnfocused.cwiseProduct(nanogui::Color(0.9f, 1.0f));
            nanogui::Color shineColor = self->theme()->mBorderLight;
            nanogui::Color shadowColor = self->theme()->mBorderDark;

            nvgSave(ctx);

            nvgTranslate(ctx, self->position().x(), self->position().y());
            nvgBeginPath(ctx);
            nvgRoundedRect(ctx, 0, 0, self->width(), self->height(), 1.0f);
            nvgFillColor(ctx, fillColor);
            nvgFill(ctx);

            nvgRestore(ctx);

            self->Widget::draw(ctx);

            nvgSave(ctx);


            nvgTranslate(ctx, self->position().x(), self->position().y());
            nvgBeginPath(ctx);
            nvgMoveTo(ctx, 1.25f, self->height());
            nvgLineTo(ctx, 1.25f, 1.5f);
            nvgLineTo(ctx, self->width(), 1.5f);
            nvgStrokeColor(ctx, shineColor);
            nvgStrokeWidth(ctx, 1.0f);
            nvgLineJoin(ctx, NVG_ROUND);
            nvgStroke(ctx);

            nvgBeginPath(ctx);
            nvgMoveTo(ctx, 0.75f, self->height());
            nvgLineTo(ctx, 0.75f, 0.5f);
            nvgLineTo(ctx, self->width(), 0.5f);
            nvgStrokeColor(ctx, shadowColor);
            nvgStrokeWidth(ctx, 1.0f);
            nvgLineJoin(ctx, NVG_ROUND);
            nvgStroke(ctx);

            nvgRestore(ctx);
        });

    /* Create circuit widget in right pane. */
    m_circuit = new CircuitWidget(m_sidePanelR, m_window, m_unitEditorHost, a_vm, a_uf);

    /* Create button panel */
    m_buttonPanel = new EnhancedWindow(m_screen, "");
    m_buttonPanel->setIsBackgroundWindow(true);
    m_buttonPanel->setDrawCallback([](EnhancedWindow* self, NVGcontext* ctx)
        {
            nanogui::Color fillColor = self->theme()->mWindowFillUnfocused.cwiseProduct(nanogui::Color(0.9f, 1.0f));

            nvgSave(ctx);

            nvgTranslate(ctx, self->position().x(), self->position().y());
            nvgBeginPath(ctx);
            nvgRoundedRect(ctx, 0, 0, self->width(), self->height(), 1.0f);
            nvgFillColor(ctx, fillColor);
            nvgFill(ctx);

            nvgRestore(ctx);

            self->Widget::draw(ctx);
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
    createLogViewer_(new nanogui::Widget(logScrollPanel));

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
    m_settingsFormHelper = std::make_shared<synui::SerializableFormHelper>(m_screen);
    createSettingsEditor_(new nanogui::Widget(settingsScrollPanel), m_settingsFormHelper.get());

    /* Add a button for openning the settings editor window. */
    buttonPanelLayout->appendCol(0, 0);
    auto settings_callback = [this]()
            {
                m_settingsFormHelper->refresh();
                m_screen->centerWindow(m_settingsEditor);
            };
    auto settings_button = m_settingsEditor->createOpenButton(m_buttonPanel, "", ENTYPO_COG, settings_callback);
    buttonPanelLayout->setAnchor(settings_button, nanogui::AdvancedGridLayout::Anchor{buttonPanelLayout->colCount() - 1,0});

    m_screen->performLayout();
}

void synui::MainGUI::show()
{
    m_screen->setVisible(true);
    m_screen->performLayout();
}

void synui::MainGUI::hide() { m_screen->setVisible(false); }

void synui::MainGUI::draw() { m_screen->drawAll(); }

synui::MainGUI::~MainGUI()
{
    Logger::instance().reset();
}
