#ifndef SF_UI_H
#define SF_UI_H

class UI_Element
{
    public:
        UI_Element(int32 x, int32 y, uint32 width, uint32 height);

        virtual void Draw() { };

        void Delete();
        bool IsBeingDeleted();

        void SetPosition(int32 x, int32 y)
        {
            m_positionX = x;
            m_positionY = y;

            // TODO: move event
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
            m_width = width;
            m_height = height;

            // TODO: resize event
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
        virtual void onResize() {};
        virtual void onMove() {};

    protected:
        bool m_toDelete;

        int32 m_positionX;
        int32 m_positionY;
        uint32 m_width;
        uint32 m_height;
};

#define SELF UI_Button

class UIManager;

class UI_Button: public UI_Element
{
    friend bool UIManager::MouseClick(uint32 x, uint32 y);

    public:
        UI_Button(int32 x, int32 y,
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

#endif
