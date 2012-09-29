#ifndef SIMPLYFLAT_MAIN_HEADER
#define SIMPLYFLAT_MAIN_HEADER

// TODO: multiplatform macro
#include <windows.h>
#include <cmath>

#include <vector>
#include <map>

#include <Defines.h>
#include <Text.h>

#include <gl\gl.h>
#include <gl\glu.h>

#define M_PI 3.1415926f

#define KEY_COUNT 256

enum MouseButtons
{
    MOUSE_BUTTON_LEFT  = 0,
    MOUSE_BUTTON_RIGHT = 1,
    MOUSE_BUTTON_MAX
};

#define COLOR(r,g,b) (uint32(r) << 16 | uint32(g) << 8 | uint32(b))

class SimplyFlat
{
    public:
        SimplyFlat();
        ~SimplyFlat();

        bool CreateMainWindow(const char* title, uint32 width, uint32 height, uint8 colordepth, bool fullscreen = false, uint32 refreshrate = 60, LRESULT (CALLBACK *WndProc)(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) = NULL);
        void DestroyMainWindow();
        void ResizeMainWindow(uint32 width, uint32 height);
        void InitDrawing();
        int32 BuildFont(const char* fontFile, uint32 height, uint16 bold = FW_DONTCARE, bool italic = false, bool underline = false, bool strikeout = false);
        LRESULT CALLBACK SFWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

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
                void DrawRectangle(uint32 top, uint32 left, uint32 width, uint32 height, uint32 color);
                void DrawCircle(uint32 center_x, uint32 center_y, float radius, uint32 color);
                void ClearColor(uint8 r, uint8 g, uint8 b);
                void PrintText(uint32 fontId, uint32 x, uint32 y, const char *fmt, ...);

                uint32 fontDataMapSize() { return m_fontDataMap.size(); }
                void SetFontData(uint32 id, fontData* data) { if (fontDataMapSize() <= id) m_fontDataMap.resize(id+1); m_fontDataMap[id] = data; }

            private:
                std::vector<fontData*> m_fontDataMap;
        } *Drawing;

    private:
        HDC       hDC;
        HGLRC     hRC;
        HWND      hWnd;
        HINSTANCE hInstance;
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
