#ifndef SF_UI_H
#define SF_UI_H

class UI_Element
{
    public:
        UI_Element(const char* name, int32 x, int32 y, uint32 width, uint32 height);

        virtual void Draw() { };

        void Delete();
        bool IsBeingDeleted();

        void SetName(const char* name)
        {
            m_elementName = name;
        }

        const char* GetName()
        {
            return m_elementName.c_str();
        }

        void SetPosition(int32 x, int32 y)
        {
            int32 ox = m_positionX;
            int32 oy = m_positionY;

            m_positionX = x;
            m_positionY = y;

            onMove(ox, oy, x, y);
        }

        int32 GetPositionX()
        {
            return m_positionX;
        }

        int32 GetPositionY()
        {
            return m_positionY;
        }

        void SetSize(uint32 width, uint32 height)
        {
            uint32 oldw = m_width;
            uint32 oldh = m_height;

            m_width = width;
            m_height = height;

            onResize(oldw, oldh, width, height);
        }

        uint32 GetWidth()
        {
            return m_width;
        }

        uint32 GetHeight()
        {
            return m_height;
        }

        virtual void onClick() {};
        virtual void onFocus() {};
        virtual void onFocusLost() {};
        virtual void onKeyPress(uint16 key, bool press, bool printable) {};

        virtual void onResize(uint32 oldWidth, uint32 oldHeight, uint32 newWidth, uint32 newHeight) {};
        virtual void onMove(int32 oldX, int32 oldY, int32 newX, int32 newY) {};

    protected:
        bool m_toDelete;

        int32 m_positionX;
        int32 m_positionY;
        uint32 m_width;
        uint32 m_height;

        std::string m_elementName;
};

class UIManager;

#define SELF UI_Button

class UI_Button: public UI_Element
{
    friend bool UIManager::MouseClick(uint32 x, uint32 y);

    public:
        UI_Button(const char* name, int32 x, int32 y,
            const wchar_t* text = NULL,
            int32 font = 0,
            uint32 width = 0, uint32 height = 0,
            uint32 bgColor = MAKE_COLOR_RGBA(255,255,255,255),
            uint32 textColor = MAKE_COLOR_RGBA(0,0,0,255));

        void Draw();

        SELF* SetBorder(uint32 color, uint32 width);
        SELF* SetBorder(uint32 color, uint32 width, bool plastic);
        SELF* SetText(const wchar_t* text);
        SELF* SetPadding(uint32 horizontal, uint32 vertical);

        SELF* SetBackgroundColor(uint32 color);
        SELF* SetTextColor(uint32 color);

        SELF* SetOnClickCallback(void (*click)(UI_Element*));

        static const uint32 DEFAULT_PADDING = 5;

        void onClick();

    protected:
        uint32 m_horizontalPadding;
        uint32 m_verticalPadding;
        std::wstring m_text;
        uint32 m_minWidth;
        uint32 m_minHeight;

        struct
        {
            void (*onClick)(UI_Element*);
        } m_callbacks;

        struct
        {
            uint32 background;
            uint32 text;
        } m_colors;

        struct
        {
            uint32 width;
            uint32 color;
            bool plastic;
        } m_border;

        struct
        {
            uint32 textWidth;
            uint32 textHeight;
            int32  textFont;
        } m_cached;

    private:

        void RecalculateSize();
        void ProcessCaching();
};

#undef SELF

#define SELF UI_Textbox

class UI_Textbox: public UI_Element
{
    public:
        UI_Textbox(const char* name, int32 x, int32 y,
            const wchar_t* text = NULL,
            int32 font = 0,
            uint32 width = 0, uint32 height = 0,
            uint32 bgColor = MAKE_COLOR_RGBA(255,255,255,255),
            uint32 textColor = MAKE_COLOR_RGBA(0,0,0,255));

        void Draw();

        SELF* SetBorder(uint32 color, uint32 width);
        SELF* SetBorder(uint32 color, uint32 width, bool plastic);
        SELF* SetText(const wchar_t* text);
        const wchar_t* GetText();
        SELF* SetPadding(uint32 horizontal, uint32 vertical);

        SELF* SetBackgroundColor(uint32 color);
        SELF* SetTextColor(uint32 color);

        SELF* SetPasswordChar(const wchar_t passwordchar);

        //SELF* SetOnClickCallback(void (*click)(UI_Element*));

        static const uint32 DEFAULT_PADDING = 5;

        void onClick();
        void onKeyPress(uint16 key, bool press, bool printable);

    protected:
        uint32 m_horizontalPadding;
        uint32 m_verticalPadding;
        std::wstring m_text;
        uint32 m_minWidth;
        uint32 m_minHeight;
        wchar_t m_passwordChar;

        /*struct
        {
            void (*onClick)(UI_Element*);
        } m_callbacks;*/

        struct
        {
            uint32 background;
            uint32 text;
        } m_colors;

        struct
        {
            uint32 width;
            uint32 color;
            bool plastic;
        } m_border;

        struct
        {
            uint32 printedChars;
            uint32 textHeight;
            int32  textFont;
            std::wstring printedString;
        } m_cached;

    private:

        void ProcessCaching();
};

#undef SELF

#define SELF UI_Label

class UI_Label: public UI_Element
{
    public:
        UI_Label(const char* name, int32 x, int32 y,
            const wchar_t* text = NULL,
            int32 font = 0,
            uint32 textColor = MAKE_COLOR_RGBA(255,255,255,255));

        void Draw();

        SELF* SetText(const wchar_t* text);
        const wchar_t* GetText();

        SELF* SetTextColor(uint32 color);

    protected:
        std::wstring m_text;
        int32 m_textFont;
        uint32 m_textColor;

    private:
        //
};

#undef SELF

#endif
