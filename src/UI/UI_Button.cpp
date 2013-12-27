#include <SimplyFlat.h>
#include <UIManager.h>
#include <UI.h>

UI_Button::UI_Button(const char* name, int32 x, int32 y, const wchar_t* text, int32 font, uint32 width, uint32 height, uint32 bgColor, uint32 textColor):
    UI_Element(name, x, y, width, height)
{
    memset(&m_cached, 0, sizeof(m_cached));
    memset(&m_border, 0, sizeof(m_border));
    memset(&m_colors, 0, sizeof(m_colors));
    memset(&m_callbacks, 0, sizeof(m_callbacks));

    m_minWidth = width;
    m_minHeight = height;

    m_horizontalPadding = DEFAULT_PADDING;
    m_verticalPadding = DEFAULT_PADDING;

    m_text = text;
    m_cached.textFont = font;

    SetBackgroundColor(bgColor);
    SetTextColor(textColor);

    RecalculateSize();
}

void UI_Button::ProcessCaching()
{
    m_cached.textWidth = SF->Drawing->GetTextWidth(m_cached.textFont, 0, m_text.c_str());
    m_cached.textHeight = SF->Drawing->GetFontHeight(m_cached.textFont);
}

void UI_Button::RecalculateSize()
{
    ProcessCaching();

    m_width = m_cached.textWidth + 2*m_horizontalPadding;
    if (m_width < m_minWidth)
        m_width = m_minWidth;

    m_height = m_cached.textHeight + 2*m_verticalPadding;
    if (m_height < m_minHeight)
        m_height = m_minHeight;
}

UI_Button* UI_Button::SetBorder(uint32 color, uint32 width)
{
    m_border.color = color;
    m_border.width = width;

    return this;
}

UI_Button* UI_Button::SetBorder(uint32 color, uint32 width, bool plastic)
{
    SetBorder(color, width);
    m_border.plastic = plastic;

    return this;
}

UI_Button* UI_Button::SetText(const wchar_t* text)
{
    m_text = text;
    RecalculateSize();

    return this;
}

UI_Button* UI_Button::SetPadding(uint32 horizontal, uint32 vertical)
{
    m_horizontalPadding = horizontal;
    m_verticalPadding = vertical;
    RecalculateSize();

    return this;
}

UI_Button* UI_Button::SetBackgroundColor(uint32 color)
{
    m_colors.background = color;

    return this;
}

UI_Button* UI_Button::SetTextColor(uint32 color)
{
    m_colors.text = color;

    return this;
}

void UI_Button::onClick()
{
    if (m_callbacks.onClick)
        m_callbacks.onClick(this);
}

UI_Button* UI_Button::SetOnClickCallback(void (*click)(UI_Element*))
{
    m_callbacks.onClick = click;

    return this;
}

void UI_Button::Draw()
{
    //if (m_border.width > 0)
    {
        //if (!m_border.plastic)
        {
            SF->Drawing->DrawRectangle(m_positionX-m_border.width, m_positionY-m_border.width, m_width+2*m_border.width, m_height+2*m_border.width, m_border.color, 0);
            SF->Drawing->DrawRectangle(m_positionX, m_positionY, m_width, m_height, m_colors.background, 0);
            glColor4ub(GET_COLOR_R(m_colors.text),GET_COLOR_G(m_colors.text),GET_COLOR_B(m_colors.text), 255);
            SF->Drawing->PrintText(m_cached.textFont, m_positionX+(m_width/2)-(m_cached.textWidth/2), m_positionY+(m_height/2)-(m_cached.textHeight/2), 0, WW_NO_WRAP, m_text.c_str());
        }
        //else
        //{
            // TODO: plastic border
        //}
    }
}
