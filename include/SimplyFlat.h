#ifndef SIMPLYFLAT_MAIN_HEADER
#define SIMPLYFLAT_MAIN_HEADER

#ifdef _WIN32
  #include <windows.h>
#endif
#include <cmath>

#include <string>
#include <algorithm>
#include <vector>
#include <map>
#include <list>

#include <Defines.h>
#include <Text.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include "../dep/SOIL/SOIL.h"

#ifndef _WIN32
  #include <GL/freeglut.h>
#endif

#ifndef M_PI
  #define M_PI 3.1415926f
#endif

#define KEY_COUNT 256

enum MouseButtons
{
    MOUSE_BUTTON_LEFT  = 0,
    MOUSE_BUTTON_RIGHT = 1,
    MOUSE_BUTTON_MAX
};

enum CustomImageLoadFlags
{
    IMAGELOAD_GREYSCALE  = 1 << 11,
};

#define MAKE_COLOR_RGB(r,g,b) ((uint32(r) << 24) | (uint32(uint8(g)) << 16) | (uint32(uint8(b)) << 8))
#define MAKE_COLOR_RGBA(r,g,b,a) ((uint32(r) << 24) | (uint32(uint8(g)) << 16) | (uint32(uint8(b)) << 8) | (uint8(a)))
#define GET_COLOR_R(c) uint8(c >> 24)
#define GET_COLOR_G(c) uint8(c >> 16)
#define GET_COLOR_B(c) uint8(c >> 8)
#define GET_COLOR_A(c) uint8(c)

class SimplyFlat
{
    public:
        SimplyFlat();
        ~SimplyFlat();

#ifdef _WIN32
        bool CreateMainWindow(const char* title, uint32 width, uint32 height, uint8 colordepth, bool fullscreen = false, uint32 refreshrate = 60, LRESULT (CALLBACK *WndProc)(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) = NULL);
#else
        bool CreateMainWindow(const char* title, uint32 width, uint32 height, uint8 colordepth, bool fullscreen = false, uint32 refreshrate = 60, void (*drawingcallback)() = NULL);
        void Run();
#endif

        void DestroyMainWindow();
        void ResizeMainWindow(uint32 width, uint32 height);
        void InitDrawing();
        int32 BuildFont(const char* fontFile, uint32 height, uint16 bold = FW_DONTCARE, bool italic = false, bool underline = false, bool strikeout = false);

        uint32 GetScreenWidth() { return m_screenWidth; };
        uint32 GetScreenHeight() { return m_screenHeight; };

#ifdef _WIN32
        LRESULT CALLBACK SFWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#else
        //
#endif

        void BeforeDraw();
        void AfterDraw();

        // Interface struct
        struct t_Interface
        {
            public:
                t_Interface();
                ~t_Interface();
                bool IsKeyPressed(uint16 key) { if (key < KEY_COUNT) return keys[key]; else return false; };
                bool IsMouseButtonPressed(MouseButtons button) { return mousebuttons[button]; };
                bool HasHookedEvent(uint16 key) { if (key < KEY_COUNT) return (handlers[key] != NULL); else return true; };
                bool HasHookedMouseEvent() { return mousehandler != NULL; };
                void HookEvent(uint16 key, void (*handler)(uint16,bool));
                void HookMouseEvent(void (*handler)(bool,bool));
                void KeyEvent(uint16 key, bool press);

                void MouseEvent(bool left, bool press);

            private:
                bool keys[KEY_COUNT];
                bool mousebuttons[MOUSE_BUTTON_MAX];
                void (*handlers[KEY_COUNT])(uint16,bool);
                void (*mousehandler)(bool,bool);
                void (*allKeyHandler)(uint16,bool);
        } *Interface;

        // Drawing struct
        struct t_Drawing
        {
            public:
                void DrawRectangle(uint32 x, uint32 y, uint32 width, uint32 height, uint32 color, uint32 texture = 0);
                void DrawCircle(uint32 center_x, uint32 center_y, float radius, uint32 color);
                void ClearColor(uint8 r, uint8 g, uint8 b);
                void PrintText(uint32 fontId, uint32 x, uint32 y, uint8 feature, int32 wordWrapLimit, const wchar_t *fmt, ...);
                void PrintStyledText(uint32 x, uint32 y, int32 wordWrapLimit, StyledTextList* printList);

                uint32 fontDataMapSize() { return m_fontDataMap.size(); }
                void SetFontData(uint32 id, fontData* data) { if (fontDataMapSize() <= id) m_fontDataMap.resize(id+1); m_fontDataMap[id] = data; }

            private:
                std::vector<fontData*> m_fontDataMap;
        } *Drawing;

        struct t_TextureStorage
        {
            public:
                uint32 LoadTexture(const char* path, uint32 flags);
                uint32 GetGLTextureID(uint32 id);

            private:
                std::vector<uint32> m_textureMap;
        } *TextureStorage;

    private:

#ifdef _WIN32
        // On Windows, we store this kind of stuff for later reuse
        HDC       hDC;
        HGLRC     hRC;
        HWND      hWnd;
        HINSTANCE hInstance;
#endif

        uint32    m_screenWidth;
        uint32    m_screenHeight;
        bool      m_fullscreen;
};

template <class T>
class SimplyFlatSingleton
{
    public:
        SimplyFlatSingleton()
        {
            m_instance = NULL;
        }
        ~SimplyFlatSingleton()
        {
            if (m_instance)
                delete m_instance;
            m_instance = NULL;
        }

        static T* instance()
        {
            if (m_instance)
                return m_instance;

            m_instance = new T;
            return m_instance;
        }

    private:
        static T* m_instance;
};

template <class T> T* SimplyFlatSingleton<T>::m_instance = NULL;

#define sSimplyFlat SimplyFlatSingleton<SimplyFlat>::instance()

#define SF sSimplyFlat
#define SFDrawing SF->Drawing
#define SFInterface SF->Interface

#endif
