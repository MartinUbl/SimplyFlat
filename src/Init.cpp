#include <Defines.h>
#include <SimplyFlat.h>

SimplyFlat::SimplyFlat()
{
    Interface = new t_Interface;
    Drawing = new t_Drawing;
    TextureStorage = new t_TextureStorage;
}

SimplyFlat::~SimplyFlat()
{
    if (Interface)
        delete Interface;

    if (Drawing)
        delete Drawing;

    if (TextureStorage)
        delete TextureStorage;
}

void SimplyFlat::InitDrawing()
{
    glEnable(GL_TEXTURE_2D);
    glShadeModel(GL_SMOOTH);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0f);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_COLOR_MATERIAL);

    // Init 2D
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    int vPort[4];
    glGetIntegerv(GL_VIEWPORT, vPort);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(vPort[0], vPort[0]+vPort[2], vPort[1]+vPort[3], vPort[1]);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
}

#ifdef _WIN32
LRESULT CALLBACK SimplyFlat::SFWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_SYSCOMMAND:
        {
            switch (wParam)
            {
                case SC_SCREENSAVE:
                case SC_MONITORPOWER:
                    return 0;
            }
            break;
        }

        case WM_CLOSE:
        {
            PostQuitMessage(0);
            return 0;
        }

        case WM_KEYDOWN:
        {
            Interface->KeyEvent(wParam, true);
            return 0;
        }
        case WM_KEYUP:
        {
            Interface->KeyEvent(wParam, false);
            return 0;
        }

        case WM_LBUTTONDOWN:
        {
            Interface->MouseEvent(true, true);
            return 0;
        }
        case WM_LBUTTONUP:
        {
            Interface->MouseEvent(true, false);
            return 0;
        }

        case WM_MBUTTONDOWN:
        {
            return 0;
        }
        case WM_MBUTTONUP:
        {
            return 0;
        }

        case WM_RBUTTONDOWN:
        {
            Interface->MouseEvent(false, true);
            return 0;
        }
        case WM_RBUTTONUP:
        {
            Interface->MouseEvent(false, false);
            return 0;
        }

        case WM_MOUSEMOVE:
        {
            Interface->SetMouseXY(LOWORD(lParam), HIWORD(lParam));
            return 0;
        }

        case WM_SIZE:
        {
            ResizeMainWindow(LOWORD(lParam),HIWORD(lParam));
            return 0;
        }
    }

    return DefWindowProc(hWnd,uMsg,wParam,lParam);
}
#else
//
#endif

#ifdef _WIN32
bool SimplyFlat::CreateMainWindow(const char* title, uint32 width, uint32 height, uint8 colordepth, bool fullscreen, uint32 refreshrate, LRESULT (CALLBACK *WndProc)(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam))
{
    GLuint PixelFormat;
    WNDCLASS wc;
    DWORD dwExStyle;
    DWORD dwStyle;
    RECT WindowRect;
    WindowRect.left = (long)0;
    WindowRect.right = (long)width;
    WindowRect.top = (long)0;
    WindowRect.bottom = (long)height;

    m_screenWidth = width;
    m_screenHeight = height;

    m_fullscreen = fullscreen;

    hInstance            = GetModuleHandle(NULL);
    wc.style             = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc       = (WNDPROC)(WndProc);
    wc.cbClsExtra        = 0;
    wc.cbWndExtra        = 0;
    wc.hInstance         = hInstance;
    wc.hIcon             = LoadIcon(NULL, IDI_WINLOGO);
    wc.hCursor           = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground     = NULL;
    wc.lpszMenuName      = NULL;
    wc.lpszClassName     = "OpenGL";

    if (!RegisterClass(&wc))
    {
        MessageBox(NULL,"Failed To Register The Window Class.","ERROR",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    if (fullscreen)
    {
        DEVMODE dmScreenSettings;
        memset(&dmScreenSettings,0,sizeof(dmScreenSettings));
        dmScreenSettings.dmSize          = sizeof(dmScreenSettings);
        dmScreenSettings.dmPelsWidth     = width;
        dmScreenSettings.dmPelsHeight    = height;
        dmScreenSettings.dmBitsPerPel    = colordepth;
        dmScreenSettings.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;

        if (ChangeDisplaySettings(&dmScreenSettings,CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL)
        {
            if (MessageBox(NULL,"The Requested Fullscreen Mode Is Not Supported By\nYour Video Card. Use Windowed Mode Instead?","ERROR",MB_YESNO|MB_ICONEXCLAMATION)==IDYES)
            {
                fullscreen = FALSE;
            }
            else
            {
                MessageBox(NULL,"Program Will Now Close.","ERROR",MB_OK|MB_ICONSTOP);
                return FALSE;
            }
        }
    }

    if (fullscreen)
    {
        dwExStyle = WS_EX_APPWINDOW;
        dwStyle = WS_POPUP;
    }
    else
    {
        dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
        dwStyle = WS_OVERLAPPEDWINDOW;
    }

    AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);

    if (!(hWnd = CreateWindowEx(dwExStyle,
                                "OpenGL",
                                title,
                                dwStyle |
                                WS_CLIPSIBLINGS |
                                WS_CLIPCHILDREN,
                                0, 0,
                                WindowRect.right-WindowRect.left,
                                WindowRect.bottom-WindowRect.top,
                                NULL,
                                NULL,
                                hInstance,
                                NULL)))
    {
        DestroyMainWindow();
        MessageBox(NULL,"Window Creation Error.","ERROR",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    static PIXELFORMATDESCRIPTOR pfd =
    {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW |
        PFD_SUPPORT_OPENGL |
        PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,
        colordepth,
        0, 0, 0, 0, 0, 0,
        0,
        0,
        0,
        0, 0, 0, 0,
        16,
        0,
        0,
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
    };

    if (!(hDC = GetDC(hWnd)))
    {
        DestroyMainWindow();
        MessageBox(NULL,"Can't Create A GL Device Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
        return false;
    }

    if (!(PixelFormat = ChoosePixelFormat(hDC,&pfd)))
    {
        DestroyMainWindow();
        MessageBox(NULL,"Can't Find A Suitable PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
        return false;
    }

    if (!SetPixelFormat(hDC,PixelFormat,&pfd))
    {
        DestroyMainWindow();
        MessageBox(NULL,"Can't Set The PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
        return false;
    }

    if (!(hRC = wglCreateContext(hDC)))
    {
        DestroyMainWindow();
        MessageBox(NULL,"Can't Create A GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
        return false;
    }

    if (!wglMakeCurrent(hDC,hRC))
    {
        DestroyMainWindow();
        MessageBox(NULL,"Can't Activate The GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
        return false;
    }

    ShowWindow(hWnd,SW_SHOW);
    SetForegroundWindow(hWnd);
    SetFocus(hWnd);
    ResizeMainWindow(width, height);

    InitDrawing();

    return true;
}
#else
bool SimplyFlat::CreateMainWindow(int* orig_argc, char** orig_argv, const char* title, uint32 width, uint32 height, uint8 colordepth, bool fullscreen, uint32 refreshrate, void (*drawingcallback)())
{
    glutInit(orig_argc, orig_argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(width, height);
    glutInitWindowPosition(0, 0);
    glutCreateWindow(title);

    InitDrawing();

    glutDisplayFunc(drawingcallback);

    return true;
}

void SimplyFlat::Run()
{
    glutMainLoop();
}
#endif

void SimplyFlat::DestroyMainWindow()
{
    if (m_fullscreen)
    {
#ifdef _WIN32
        ChangeDisplaySettings(NULL,0);
        ShowCursor(TRUE);
#else
        glutSetCursor(GLUT_CURSOR_LEFT_ARROW);
#endif
    }

#ifdef _WIN32
    if (hRC)
    {
        if(!wglMakeCurrent(NULL,NULL))
        {
            MessageBox(NULL,"Release Of DC And RC Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
        }

        if(!wglDeleteContext(hRC))
        {
            MessageBox(NULL,"Release Rendering Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
        }
        hRC = NULL;
    }

    if (hDC && !ReleaseDC(hWnd,hDC))
    {
        MessageBox(NULL,"Release Device Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
        hDC = NULL;
    }

    if (hWnd && !DestroyWindow(hWnd))
    {
        MessageBox(NULL,"Could Not Release hWnd.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
        hWnd = NULL;
    }

    if (!UnregisterClass("OpenGL",hInstance))
    {
        MessageBox(NULL,"Could Not Unregister Class.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
        hInstance = NULL;
    }
#endif
}

void SimplyFlat::ResizeMainWindow(uint32 width, uint32 height)
{
    glViewport(0,0,width,height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,100.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}
