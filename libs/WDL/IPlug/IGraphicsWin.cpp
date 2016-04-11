#include "IGraphicsWin.h"
#include "IControl.h"
#include "Log.h"
#include <wininet.h>
#include <Shlobj.h>
#include <commctrl.h>
#include "nanovg.h"
#include "nanovg_gl.h"

#ifdef RTAS_API
  #include "PlugInUtils.h"
#endif

#pragma warning(disable:4244) // Pointer size cast mismatch.
#pragma warning(disable:4312) // Pointer size cast mismatch.
#pragma warning(disable:4311) // Pointer size cast mismatch.

static int nWndClassReg = 0;
static const char* wndClassName = "IPlugWndClass";
static double sFPS = 0.0;

#define PARAM_EDIT_ID 99

enum EParamEditMsg
{
  kNone,
  kEditing,
  kUpdate,
  kCancel,
  kCommit
};

#define IPLUG_TIMER_ID 2

inline IMouseMod GetMouseMod(WPARAM wParam)
{
  return IMouseMod((wParam & MK_LBUTTON), 
                   (wParam & MK_RBUTTON),
                   (wParam & MK_SHIFT), 
                   (wParam & MK_CONTROL), 
                   
#ifdef RTAS_API
                   IsOptionKeyDown()
#elif defined(AAX_API)
                   GetAsyncKeyState(VK_MENU) < 0
#else
                   GetKeyState(VK_MENU) < 0
#endif
                   );
}

// static
LRESULT CALLBACK IGraphicsWin::ParamEditProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  IGraphicsWin* pGraphics = (IGraphicsWin*) GetWindowLongPtr(GetParent(hWnd), GWLP_USERDATA);

  if (pGraphics && pGraphics->mParamEditWnd && pGraphics->mParamEditWnd == hWnd)
  {
    pGraphics->HideTooltip();

    switch (msg)
    {
      case WM_CHAR:
      {
        // limit to numbers for text entry on appropriate parameters
        if(pGraphics->mEdParam)
        {
          char c = wParam;

          if(c == 0x08) break; // backspace

          switch ( pGraphics->mEdParam->Type() )
          {
            case IParam::kTypeEnum:
            case IParam::kTypeInt:
            case IParam::kTypeBool:
              if (c >= '0' && c <= '9') break;
              else if (c == '-') break;
              else if (c == '+') break;
              else return 0;
            case IParam::kTypeDouble:
              if (c >= '0' && c <= '9') break;
              else if (c == '-') break;
              else if (c == '+') break;
              else if (c == '.') break;
              else return 0;
            default:
              break;
          }
        }
        break;
      }
      case WM_KEYDOWN:
      {
        if (wParam == VK_RETURN)
        {
          pGraphics->mParamEditMsg = kCommit;
          return 0;
        }
        else if (wParam == VK_ESCAPE)
        {
          pGraphics->mParamEditMsg = kCancel;
          return 0;
        }
        break;
      }
      case WM_SETFOCUS:
      {
        pGraphics->mParamEditMsg = kEditing;
        break;
      }
      case WM_KILLFOCUS:
      {
        pGraphics->mParamEditMsg = kCommit;
        break;
      }
      // handle WM_GETDLGCODE so that we can say that we want the return key message
      //  (normally single line edit boxes don't get sent return key messages)
      case WM_GETDLGCODE:
      {
        if (pGraphics->mEdParam) break;
        LPARAM lres;
        // find out if the original control wants it
        lres = CallWindowProc(pGraphics->mDefEditProc, hWnd, WM_GETDLGCODE, wParam, lParam);
        // add in that we want it if it is a return keydown
        if (lParam && ((MSG*)lParam)->message == WM_KEYDOWN  &&  wParam == VK_RETURN)
        {
          lres |= DLGC_WANTMESSAGE;
        }
        return lres;
      }
      case WM_COMMAND:
      {
        switch HIWORD(wParam)
        {
          case CBN_SELCHANGE:
          {
            if (pGraphics->mParamEditWnd)
            {
              pGraphics->mParamEditMsg = kCommit;
              return 0;
            }
          }

        }
        break;  // Else let the default proc handle it.
      }
    }
    return CallWindowProc(pGraphics->mDefEditProc, hWnd, msg, wParam, lParam);
  }
  return DefWindowProc(hWnd, msg, wParam, lParam);
}

IGraphicsWin::IGraphicsWin(IPlugBase* pPlug, int w, int h, int refreshFPS)
  : IGraphics(pPlug, w, h, refreshFPS), mHInstance(0), mPlugWnd(0),
    mParamEditWnd(0), mTooltipWnd(0), mEdControl(0), mEdParam(0), mDefEditProc(0),
    mParamEditMsg(kNone), mShowingTooltip(false), mTooltipIdx(-1), mCustomColorStorage(0),
    mPID(0), mParentWnd(0), mMainWnd(0)
{}

IGraphicsWin::~IGraphicsWin()
{
  IGraphicsWin::CloseWindow();
  FREE_NULL(mCustomColorStorage);
}

LICE_IBitmap* IGraphicsWin::OSLoadBitmap(int ID, const char* name)
{
  const char* ext = name+strlen(name)-1;
  while (ext > name && *ext != '.') --ext;
  ++ext;

  if (!stricmp(ext, "png")) return _LICE::LICE_LoadPNGFromResource(mHInstance, ID, 0);
  #ifdef IPLUG_JPEG_SUPPORT
  if (!stricmp(ext, "jpg") || !stricmp(ext, "jpeg")) return _LICE::LICE_LoadJPGFromResource(mHInstance, ID, 0);
  #endif

  return 0;
}

void GetWindowSize(HWND pWnd, int* pW, int* pH)
{
  if (pWnd)
  {
    RECT r;
    GetWindowRect(pWnd, &r);
    *pW = r.right - r.left;
    *pH = r.bottom - r.top;
  }
  else
  {
    *pW = *pH = 0;
  }
}

bool IsChildWindow(HWND pWnd)
{
  if (pWnd)
  {
    int style = GetWindowLong(pWnd, GWL_STYLE);
    int exStyle = GetWindowLong(pWnd, GWL_EXSTYLE);
    return ((style & WS_CHILD) && !(exStyle & WS_EX_MDICHILD));
  }
  return false;
}

void IGraphicsWin::ForceEndUserEdit()
{
  mParamEditMsg = kCancel;
}

#define SETPOS_FLAGS SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE

void IGraphicsWin::Resize(int w, int h)
{
  if (w == Width() && h == Height()) return;

  int dw = w - Width(), dh = h - Height();
  IGraphics::Resize(w, h);

  if (WindowIsOpen())
  {
    HWND pParent = 0, pGrandparent = 0;
    int plugW = 0, plugH = 0, parentW = 0, parentH = 0, grandparentW = 0, grandparentH = 0;
    GetWindowSize(mPlugWnd, &plugW, &plugH);

    if (IsChildWindow(mPlugWnd))
    {
      pParent = GetParent(mPlugWnd);
      GetWindowSize(pParent, &parentW, &parentH);

      if (IsChildWindow(pParent))
      {
        pGrandparent = GetParent(pParent);
        GetWindowSize(pGrandparent, &grandparentW, &grandparentH);
      }
    }

    SetWindowPos(mPlugWnd, 0, 0, 0, plugW + dw, plugH + dh, SETPOS_FLAGS);

    // don't want to touch the host window in VST3 or RTAS
    if(mPlug->GetAPI() != kAPIVST3 && mPlug->GetAPI() != kAPIRTAS)
    {
      if(pParent)
      {
        SetWindowPos(pParent, 0, 0, 0, parentW + dw, parentH + dh, SETPOS_FLAGS);
      }

      if(pGrandparent)
      {
        SetWindowPos(pGrandparent, 0, 0, 0, grandparentW + dw, grandparentH + dh, SETPOS_FLAGS);
      }
    }

    RECT r = { 0, 0, Width(), Height() };
    InvalidateRect(mPlugWnd, &r, FALSE);
  }
}

void IGraphicsWin::HideMouseCursor()
{
  if (!mCursorHidden)
  {
    POINT p;
    GetCursorPos(&p);

    mHiddenMousePointX = p.x;
    mHiddenMousePointY = p.y;

    ShowCursor(false);
    mCursorHidden=true;
  }
}

void IGraphicsWin::ShowMouseCursor()
{
  if (mCursorHidden)
  {
    SetCursorPos(mHiddenMousePointX, mHiddenMousePointY);
    ShowCursor(true);
    mCursorHidden=false;
  }
}

int IGraphicsWin::ShowMessageBox(const char* pText, const char* pCaption, int type)
{
  return MessageBox(GetMainWnd(), pText, pCaption, type);
}

bool IGraphicsWin::DrawScreen(IRECT* pR)
{
	  PAINTSTRUCT ps;
	  HWND hWnd = (HWND) GetWindow();
	  HDC dc = BeginPaint(hWnd, &ps);
	  BitBlt(dc, pR->L, pR->T, pR->W(), pR->H(), mDrawBitmap->getDC(), pR->L, pR->T, SRCCOPY);
	  EndPaint(hWnd, &ps);
  return true;
}

void* IGraphicsWin::OpenWindow(void* pParentWnd)
{
  int x = 0, y = 0, w = Width(), h = Height();
  mParentWnd = (HWND) pParentWnd;

  if (mPlugWnd)
  {
    RECT pR, cR;
    GetWindowRect((HWND) pParentWnd, &pR);
    GetWindowRect(mPlugWnd, &cR);
    CloseWindow();
    x = cR.left - pR.left;
    y = cR.top - pR.top;
    w = cR.right - cR.left;
    h = cR.bottom - cR.top;
  }

  if (nWndClassReg++ == 0)
  {
    WNDCLASS wndClass = { CS_DBLCLKS | CS_OWNDC , DefWindowProc , 0, 0, mHInstance, 0, LoadCursor(NULL, IDC_ARROW), 0, 0, wndClassName };
    RegisterClass(&wndClass);
  }

  sFPS = FPS();

  mPlugWnd = CreateWindow(wndClassName, "IPlug", WS_CHILD, // | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                          x, y, w, h, (HWND) pParentWnd, 0, mHInstance, this);


  if (!mPlugWnd && --nWndClassReg == 0)
  {
	  UnregisterClass(wndClassName, mHInstance);
  }

  sf::ContextSettings settings;
  settings.depthBits = 24;
  settings.stencilBits = 8;
  settings.antialiasingLevel = 2; // Optional  

  m_sfmlWindow = new sf::RenderWindow(mPlugWnd, settings);
  m_sfmlWindow->setActive(false);

  m_drawThread = std::thread([&,this] {\
	  // Make context active
	  m_sfmlWindow->setActive(true);

	// Initialize glew
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		printf("Could not init glew.\n");
		return -1;
	}
	// GLEW generates GL error because it calls glGetString(GL_EXTENSIONS), we'll consume it here.
	glGetError();

	// Setup NanoVG context
		NVGcontext* vg = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);
  
	  sf::CircleShape myshape(100.f);
	  myshape.setFillColor(sf::Color::Green);

	  sf::Time updateDelay = sf::milliseconds(16);

	  bool running = true;
	  while (running)
	  {
		  // check all the window's events that were triggered since the last iteration of the loop
		  sf::Event event;
		  while (m_sfmlWindow->pollEvent(event))
		  {
			  // "close requested" event: we close the window
			  switch (event.type) {
			  case sf::Event::Closed:
				  m_sfmlWindow->setActive(false); 
				  running = false;
			  case sf::Event::Resized: 
				  glViewport(0, 0, event.size.width, event.size.height);
			  case sf::Event::LostFocus: break;
			  case sf::Event::GainedFocus: break;
			  case sf::Event::TextEntered: break;
			  case sf::Event::KeyPressed: break;
			  case sf::Event::KeyReleased: break;
			  case sf::Event::MouseWheelMoved: break;
			  case sf::Event::MouseWheelScrolled: break;
			  case sf::Event::MouseButtonPressed: break;
			  case sf::Event::MouseButtonReleased: break;
			  case sf::Event::MouseMoved: 
				  m_cursor[0] = event.mouseMove.x;
				  m_cursor[1] = event.mouseMove.y;
			  case sf::Event::MouseEntered: break;
			  case sf::Event::MouseLeft: break;
			  case sf::Event::JoystickButtonPressed: break;
			  case sf::Event::JoystickButtonReleased: break;
			  case sf::Event::JoystickMoved: break;
			  case sf::Event::JoystickConnected: break;
			  case sf::Event::JoystickDisconnected: break;
			  case sf::Event::TouchBegan: break;
			  case sf::Event::TouchMoved: break;
			  case sf::Event::TouchEnded: break;
			  case sf::Event::SensorChanged: break;
			  default: break;
			  }
		  }


		  glClearColor(0, 0, 0, 0);
		  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		  
		  // Drop shadow
		  nvgBeginFrame(vg, m_sfmlWindow->getSize().x, m_sfmlWindow->getSize().y, 1.0);
		  nvgBeginPath(vg);
		  nvgRect(vg, 100.f, 100.f - 10, 120.f, 130.f);
		  nvgFillColor(vg, nvgRGBA(255, 255, 255, 255));
		  nvgFill(vg);
		  nvgEndFrame(vg);

//		  m_sfmlWindow->pushGLStates();
//		  m_sfmlWindow->draw(myshape);
//		  m_sfmlWindow->popGLStates();

		  m_sfmlWindow->display(); 

		  m_frameCount++;
		  sf::sleep(updateDelay);
	  } // end event-handler loop
	nvgDeleteGL3(vg);
  });

  return mPlugWnd;
}

void IGraphicsWin::CloseWindow()
{
	if (mPlugWnd)
	{
		m_sfmlWindow->close();

		if (--nWndClassReg == 0)
		{
			UnregisterClass(wndClassName, mHInstance);
		}
	}
}


#define MAX_CLASSNAME_LEN 128
void GetWndClassName(HWND hWnd, WDL_String* pStr)
{
  char cStr[MAX_CLASSNAME_LEN];
  cStr[0] = '\0';
  GetClassName(hWnd, cStr, MAX_CLASSNAME_LEN);
  pStr->Set(cStr);
}

BOOL CALLBACK IGraphicsWin::FindMainWindow(HWND hWnd, LPARAM lParam)
{
  IGraphicsWin* pGraphics = (IGraphicsWin*) lParam;
  if (pGraphics)
  {
    DWORD wPID;
    GetWindowThreadProcessId(hWnd, &wPID);
    WDL_String str;
    GetWndClassName(hWnd, &str);
    if (wPID == pGraphics->mPID && !strcmp(str.Get(), pGraphics->mMainWndClassName.Get()))
    {
      pGraphics->mMainWnd = hWnd;
      return FALSE;   // Stop enumerating.
    }
  }
  return TRUE;
}

HWND IGraphicsWin::GetMainWnd()
{
  if (!mMainWnd)
  {
    if (mParentWnd)
    {
      HWND parentWnd = mParentWnd;
      while (parentWnd)
      {
        mMainWnd = parentWnd;
        parentWnd = GetParent(mMainWnd);
      }
      GetWndClassName(mMainWnd, &mMainWndClassName);
    }
    else if (CSTR_NOT_EMPTY(mMainWndClassName.Get()))
    {
      mPID = GetCurrentProcessId();
      EnumWindows(FindMainWindow, (LPARAM) this);
    }
  }
  return mMainWnd;
}

#define TOOLWIN_BORDER_W 6
#define TOOLWIN_BORDER_H 23

IRECT IGraphicsWin::GetWindowRECT()
{
  if (mPlugWnd)
  {
    RECT r;
    GetWindowRect(mPlugWnd, &r);
    r.right -= TOOLWIN_BORDER_W;
    r.bottom -= TOOLWIN_BORDER_H;
    return IRECT(r.left, r.top, r.right, r.bottom);
  }
  return IRECT();
}

void IGraphicsWin::SetWindowTitle(char* str)
{
  SetWindowText(mPlugWnd, str);
}

IPopupMenu* IGraphicsWin::GetItemMenu(long idx, long &idxInMenu, long &offsetIdx, IPopupMenu* pMenu)
{
  long oldIDx = offsetIdx;
  offsetIdx += pMenu->GetNItems();

  if (idx < offsetIdx)
  {
    idxInMenu = idx - oldIDx;
    return pMenu;
  }

  IPopupMenu* menu = 0;

  for(int i = 0; i< pMenu->GetNItems(); i++)
  {
    IPopupMenuItem* menuItem = pMenu->GetItem(i);
    if(menuItem->GetSubmenu())
    {
      menu = GetItemMenu(idx, idxInMenu, offsetIdx, menuItem->GetSubmenu());

      if(menu)
        break;
    }
  }

  return menu;
}

HMENU IGraphicsWin::CreateMenu(IPopupMenu* pMenu, long* offsetIdx)
{
  HMENU hMenu = CreatePopupMenu();

  int flags = 0;
  long idxSubmenu = 0;
  long offset = *offsetIdx;
  long nItems = pMenu->GetNItems();
  *offsetIdx += nItems;
  long inc = 0;

  for(int i = 0; i< nItems; i++)
  {
    IPopupMenuItem* menuItem = pMenu->GetItem(i);

    if (menuItem->GetIsSeparator())
    {
      AppendMenu (hMenu, MF_SEPARATOR, 0, 0);
    }
    else
    {
      const char* str = menuItem->GetText();
      char* titleWithPrefixNumbers = 0;

      if (pMenu->GetPrefix())
      {
        titleWithPrefixNumbers = (char*)malloc(strlen(str) + 50);

        switch (pMenu->GetPrefix())
        {
          case 1:
          {
            sprintf(titleWithPrefixNumbers, "%1d: %s", i+1, str); break;
          }
          case 2:
          {
            sprintf(titleWithPrefixNumbers, "%02d: %s", i+1, str); break;
          }
          case 3:
          {
            sprintf(titleWithPrefixNumbers, "%03d: %s", i+1, str); break;
          }
        }
      }

      const char* entryText (titleWithPrefixNumbers ? titleWithPrefixNumbers : str);

      flags = MF_STRING;
      //if (nItems < 160 && pMenu->getNbItemsPerColumn () > 0 && inc && !(inc % _menu->getNbItemsPerColumn ()))
      //  flags |= MF_MENUBARBREAK;

      if (menuItem->GetSubmenu())
      {
        HMENU submenu = CreateMenu(menuItem->GetSubmenu(), offsetIdx);
        if (submenu)
        {
          AppendMenu(hMenu, flags|MF_POPUP|MF_ENABLED, (UINT_PTR)submenu, (const TCHAR*)entryText);
        }
      }
      else
      {
        if (menuItem->GetEnabled())
          flags |= MF_ENABLED;
        else
          flags |= MF_GRAYED;
        if (menuItem->GetIsTitle())
          flags |= MF_DISABLED;
        if (menuItem->GetChecked())
          flags |= MF_CHECKED;
        else
          flags |= MF_UNCHECKED;

        AppendMenu(hMenu, flags, offset + inc, entryText);
      }

      if(titleWithPrefixNumbers)
        FREE_NULL(titleWithPrefixNumbers);
    }
    inc++;
  }

  return hMenu;
}

IPopupMenu* IGraphicsWin::CreateIPopupMenu(IPopupMenu* pMenu, IRECT* pAreaRect)
{
  ReleaseMouseCapture();

  long offsetIdx = 0;
  HMENU hMenu = CreateMenu(pMenu, &offsetIdx);
  IPopupMenu* result = 0;

  if(hMenu)
  {
    POINT cPos;

    cPos.x = pAreaRect->L;
    cPos.y = pAreaRect->B;

    ClientToScreen(mPlugWnd, &cPos);

    if (TrackPopupMenu(hMenu,
                       TPM_LEFTALIGN,
                       cPos.x,
                       cPos.y,
                       0,
                       mPlugWnd,
                       0))
    {
      MSG msg;
      if (PeekMessage(&msg, mPlugWnd, WM_COMMAND, WM_COMMAND, PM_REMOVE))
      {
        if (HIWORD(msg.wParam) == 0)
        {
          long res = LOWORD(msg.wParam);
          if (res != -1)
          {
            long idx = 0;
            offsetIdx = 0;
            IPopupMenu* resultMenu = GetItemMenu(res, idx, offsetIdx, pMenu);
            if(resultMenu)
            {
              result = resultMenu;
              result->SetChosenItemIdx(idx);
            }
          }
        }
      }
    }
    DestroyMenu(hMenu);

    RECT r = { 0, 0, Width(), Height() };
    InvalidateRect(mPlugWnd, &r, FALSE);
  }
  return result;
}

void IGraphicsWin::CreateTextEntry(IControl* pControl, IText* pText, IRECT* pTextRect, const char* pString, IParam* pParam)
{
  if (!pControl || mParamEditWnd) return;

  DWORD editStyle;

  switch ( pText->mAlign )
  {
    case IText::kAlignNear:   editStyle = ES_LEFT;   break;
    case IText::kAlignFar:    editStyle = ES_RIGHT;  break;
    case IText::kAlignCenter:
    default:                  editStyle = ES_CENTER; break;
  }

  mParamEditWnd = CreateWindow("EDIT", pString, ES_AUTOHSCROLL /*only works for left aligned text*/ | WS_CHILD | WS_VISIBLE | ES_MULTILINE | editStyle,
                               pTextRect->L, pTextRect->T, pTextRect->W()+1, pTextRect->H()+1,
                               mPlugWnd, (HMENU) PARAM_EDIT_ID, mHInstance, 0);

  HFONT font = CreateFont(pText->mSize, 0, 0, 0, pText->mStyle == IText::kStyleBold ? FW_BOLD : 0, pText->mStyle == IText::kStyleItalic ? TRUE : 0, 0, 0, 0, 0, 0, 0, 0, pText->mFont);

  SendMessage(mParamEditWnd, EM_LIMITTEXT, (WPARAM) pControl->GetTextEntryLength(), 0);
  SendMessage(mParamEditWnd, WM_SETFONT, (WPARAM) font, 0);
  SendMessage(mParamEditWnd, EM_SETSEL, 0, -1);

  SetFocus(mParamEditWnd);

  mDefEditProc = (WNDPROC) SetWindowLongPtr(mParamEditWnd, GWLP_WNDPROC, (LONG_PTR) ParamEditProc);
  SetWindowLongPtr(mParamEditWnd, GWLP_USERDATA, 0xdeadf00b);

  //DeleteObject(font);

  mEdControl = pControl;
  mEdParam = pParam; // could be 0
}

#define MAX_PATH_LEN 256

void GetModulePath(HMODULE hModule, WDL_String* pPath)
{
  pPath->Set("");
  char pathCStr[MAX_PATH_LEN];
  pathCStr[0] = '\0';
  if (GetModuleFileName(hModule, pathCStr, MAX_PATH_LEN))
  {
    int s = -1;
    for (int i = 0; i < strlen(pathCStr); ++i)
    {
      if (pathCStr[i] == '\\')
      {
        s = i;
      }
    }
    if (s >= 0 && s + 1 < strlen(pathCStr))
    {
      pPath->Set(pathCStr, s + 1);
    }
  }
}

void IGraphicsWin::HostPath(WDL_String* pPath)
{
  GetModulePath(0, pPath);
}

void IGraphicsWin::PluginPath(WDL_String* pPath)
{
  GetModulePath(mHInstance, pPath);
}

void IGraphicsWin::DesktopPath(WDL_String* pPath)
{
  #ifndef __MINGW_H // TODO: alternative for gcc?
  TCHAR strPath[MAX_PATH_LEN];
  SHGetSpecialFolderPath( 0, strPath, CSIDL_DESKTOP, FALSE );
  pPath->Set(strPath, MAX_PATH_LEN);
  #endif
}

void IGraphicsWin::AppSupportPath(WDL_String* pPath, bool isSystem)
{
#ifndef __MINGW_H // TODO: alternative for gcc?
  TCHAR strPath[MAX_PATH_LEN];

  if (isSystem)
    SHGetFolderPathA(NULL, CSIDL_COMMON_APPDATA, NULL, 0, strPath);
  else
    SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, strPath);

  pPath->Set(strPath, MAX_PATH_LEN);
#endif
}

//void IGraphicsWin::VST3PresetsPath(WDL_String* pPath, bool isSystem)
//{
//  TCHAR strPath[MAX_PATH_LEN];
//
//  if (!isSystem)
//  {
//    TCHAR strPath[MAX_PATH_LEN];
//    SHGetFolderPathA(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, strPath);
//    pPath->Set(strPath, MAX_PATH_LEN);
//  }
//  else
//  {
//    SHGetFolderPathA(NULL, CSIDL_COMMON_APPDATA, NULL, 0, strPath);
//    pPath->Set(strPath, MAX_PATH_LEN);
//  }
//
//  pPath->AppendFormatted(MAX_PATH_LEN, "\\VST3 Presets\\%s\\%s", mPlug->GetMfrNameStr(), mPlug->GetPluginNameStr());
//}

void IGraphicsWin::PromptForFile(WDL_String* pFilename, EFileAction action, WDL_String* pDir, char* extensions)
{
  if (!WindowIsOpen())
  {
    pFilename->Set("");
    return;
  }

  char fnCStr[MAX_PATH_LEN];
  char dirCStr[MAX_PATH_LEN];

  if (pFilename->GetLength())
    strcpy(fnCStr, pFilename->Get());
  else
    fnCStr[0] = '\0';

  dirCStr[0] = '\0';

  if (!pDir->GetLength())
  {
    DesktopPath(pDir);
  }

  strcpy(dirCStr, pDir->Get());

  OPENFILENAME ofn;
  memset(&ofn, 0, sizeof(OPENFILENAME));

  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner = mPlugWnd;
  ofn.lpstrFile = fnCStr;
  ofn.nMaxFile = MAX_PATH_LEN - 1;
  ofn.lpstrInitialDir = dirCStr;
  ofn.Flags = OFN_PATHMUSTEXIST;

  if (CSTR_NOT_EMPTY(extensions))
  {
    char extStr[256];
    char defExtStr[16];
    int i, p, n = strlen(extensions);
    bool seperator = true;

    for (i = 0, p = 0; i < n; ++i)
    {
      if (seperator)
      {
        if (p)
        {
          extStr[p++] = ';';
        }
        seperator = false;
        extStr[p++] = '*';
        extStr[p++] = '.';
      }

      if (extensions[i] == ' ')
      {
        seperator = true;
      }
      else
      {
        extStr[p++] = extensions[i];
      }
    }
    extStr[p++] = '\0';

    strcpy(&extStr[p], extStr);
    extStr[p + p] = '\0';
    ofn.lpstrFilter = extStr;

    for (i = 0, p = 0; i < n && extensions[i] != ' '; ++i)
    {
      defExtStr[p++] = extensions[i];
    }

    defExtStr[p++] = '\0';
    ofn.lpstrDefExt = defExtStr;
  }

  bool rc = false;
  switch (action)
  {
    case kFileSave:
      ofn.Flags |= OFN_OVERWRITEPROMPT;
      rc = GetSaveFileName(&ofn);
      break;

    case kFileOpen:
    default:
      ofn.Flags |= OFN_FILEMUSTEXIST;
      rc = GetOpenFileName(&ofn);
      break;
  }

  if (rc)
  {
    char drive[_MAX_DRIVE];
    #ifndef __MINGW_H // TODO: alternative for gcc
    if(_splitpath_s(ofn.lpstrFile, drive, sizeof(drive), dirCStr, sizeof(dirCStr), NULL, 0, NULL, 0) == 0)
    {
      pDir->SetFormatted(MAX_PATH_LEN, "%s%s", drive, dirCStr);
    }
    #endif
    pFilename->Set(ofn.lpstrFile);
  }
  else
  {
    pFilename->Set("");
  }
}

UINT_PTR CALLBACK CCHookProc(HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
  if (uiMsg == WM_INITDIALOG && lParam)
  {
    CHOOSECOLOR* cc = (CHOOSECOLOR*) lParam;
    if (cc && cc->lCustData)
    {
      char* str = (char*) cc->lCustData;
      SetWindowText(hdlg, str);
    }
  }
  return 0;
}

bool IGraphicsWin::PromptForColor(IColor* pColor, char* prompt)
{
  if (!mPlugWnd)
  {
    return false;
  }
  if (!mCustomColorStorage)
  {
    mCustomColorStorage = (COLORREF*) calloc(16, sizeof(COLORREF));
  }
  CHOOSECOLOR cc;
  memset(&cc, 0, sizeof(CHOOSECOLOR));
  cc.lStructSize = sizeof(CHOOSECOLOR);
  cc.hwndOwner = mPlugWnd;
  cc.rgbResult = RGB(pColor->R, pColor->G, pColor->B);
  cc.lpCustColors = mCustomColorStorage;
  cc.lCustData = (LPARAM) prompt;
  cc.lpfnHook = CCHookProc;
  cc.Flags = CC_RGBINIT | CC_ANYCOLOR | CC_FULLOPEN | CC_SOLIDCOLOR | CC_ENABLEHOOK;

  if (ChooseColor(&cc))
  {
    pColor->R = GetRValue(cc.rgbResult);
    pColor->G = GetGValue(cc.rgbResult);
    pColor->B = GetBValue(cc.rgbResult);
    return true;
  }
  return false;
}

#define MAX_INET_ERR_CODE 32
bool IGraphicsWin::OpenURL(const char* url,
                           const char* msgWindowTitle, const char* confirmMsg, const char* errMsgOnFailure)
{
  if (confirmMsg && MessageBox(mPlugWnd, confirmMsg, msgWindowTitle, MB_YESNO) != IDYES)
  {
    return false;
  }
  DWORD inetStatus = 0;
  if (InternetGetConnectedState(&inetStatus, 0))
  {
    if ((int) ShellExecute(mPlugWnd, "open", url, 0, 0, SW_SHOWNORMAL) > MAX_INET_ERR_CODE)
    {
      return true;
    }
  }
  if (errMsgOnFailure)
  {
    MessageBox(mPlugWnd, errMsgOnFailure, msgWindowTitle, MB_OK);
  }
  return false;
}

void IGraphicsWin::SetTooltip(const char* tooltip)
{
  TOOLINFO ti = { TTTOOLINFOA_V2_SIZE, 0, mPlugWnd, (UINT_PTR)mPlugWnd };
  ti.lpszText = (LPTSTR)tooltip;
  SendMessage(mTooltipWnd, TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);
}

void IGraphicsWin::ShowTooltip()
{
  const char* tooltip = GetControl(mTooltipIdx)->GetTooltip();
  if (tooltip)
  {
    assert(strlen(tooltip) < 80);
    SetTooltip(tooltip);
    mShowingTooltip = true;
  }
}

void IGraphicsWin::HideTooltip()
{
  if (mShowingTooltip)
  {
    SetTooltip(NULL);
    mShowingTooltip = false;
  }
}

bool IGraphicsWin::GetTextFromClipboard(WDL_String* pStr)
{
  bool success = false;
  HGLOBAL hglb;
  
  if (IsClipboardFormatAvailable(CF_UNICODETEXT))
  {
    if(OpenClipboard(0))
    {
      hglb = GetClipboardData(CF_UNICODETEXT);
      
      if(hglb != NULL)
      {
        WCHAR *orig_str = (WCHAR*)GlobalLock(hglb);
        
        if (orig_str != NULL)
        {
          int orig_len = (int) wcslen(orig_str);
          
          orig_len += 1;
          
          // find out how much space is needed
          int new_len = WideCharToMultiByte(CP_UTF8,
                                            0,
                                            orig_str,
                                            orig_len,
                                            0,
                                            0,
                                            NULL,
                                            NULL);
          
          if (new_len > 0)
          {
            char *new_str = new char[new_len + 1];
            
            int num_chars = WideCharToMultiByte(CP_UTF8,
                                                0,
                                                orig_str,
                                                orig_len,
                                                new_str,
                                                new_len,
                                                NULL,
                                                NULL);
            
            if (num_chars > 0)
            {
              success = true;
              pStr->Set(new_str);
            }
            
            delete [] new_str;
          }
          
          GlobalUnlock(hglb);
        }
      }
    }
    
    CloseClipboard();
  }
  
  if(!success)
    pStr->Set("");
  
  return success;
}

