#include <SimplyFlat.h>
#include <UIManager.h>
#include <UI.h>

UI_Textbox::UI_Textbox(const char* name, int32 x, int32 y, const wchar_t* text, int32 font, uint32 width, uint32 height, uint32 bgColor, uint32 textColor):
    UI_Element(name, x, y, width, height)
{
    memset(&m_cached, 0, sizeof(m_cached));
    memset(&m_border, 0, sizeof(m_border));
    memset(&m_colors, 0, sizeof(m_colors));
    //memset(&m_callbacks, 0, sizeof(m_callbacks));

    m_minWidth = width;
    m_minHeight = height;

    m_horizontalPadding = DEFAULT_PADDING;
    m_verticalPadding = DEFAULT_PADDING;

    m_passwordChar = 0;

    m_cached.printedString.clear();

    SetText(text);
    m_cached.textFont = font;

    SetBackgroundColor(bgColor);
    SetTextColor(textColor);

    ProcessCaching();
}

void UI_Textbox::ProcessCaching()
{
    m_cached.textHeight = SF->Drawing->GetFontHeight(m_cached.textFont);
}

UI_Textbox* UI_Textbox::SetBorder(uint32 color, uint32 width)
{
    m_border.color = color;
    m_border.width = width;

    return this;
}

UI_Textbox* UI_Textbox::SetBorder(uint32 color, uint32 width, bool plastic)
{
    SetBorder(color, width);
    m_border.plastic = plastic;

    return this;
}

UI_Textbox* UI_Textbox::SetText(const wchar_t* text)
{
    m_text = text;

    // recalculate printable characters count

    uint32 textarea = m_width - m_border.width*2 - m_verticalPadding*2;

    if (m_passwordChar != 0)
    {
        int32 onewidth = SF->Drawing->GetTextWidth(m_cached.textFont, 0, L"%c", m_passwordChar);
        m_cached.printedChars = min(textarea / onewidth, m_text.size());

        m_cached.printedString.clear();
        for (uint32 i = 0; i < m_cached.printedChars; i++)
            m_cached.printedString += m_passwordChar;
    }
    else
    {
        m_cached.printedChars = SF->Drawing->GetTextAmountToWidthFromEnd(m_cached.textFont, 0, textarea, text);

        m_cached.printedString.clear();
        for (uint32 i = m_text.size()-m_cached.printedChars; i < m_text.size(); i++)
            m_cached.printedString += m_text[i];
    }

    return this;
}

const wchar_t* UI_Textbox::GetText()
{
    return m_text.c_str();
}

UI_Textbox* UI_Textbox::SetPasswordChar(const wchar_t passchar)
{
    m_passwordChar = passchar;
    SetText(GetText());
    return this;
}

UI_Textbox* UI_Textbox::SetPadding(uint32 horizontal, uint32 vertical)
{
    m_horizontalPadding = horizontal;
    m_verticalPadding = vertical;

    return this;
}

UI_Textbox* UI_Textbox::SetBackgroundColor(uint32 color)
{
    m_colors.background = color;

    return this;
}

UI_Textbox* UI_Textbox::SetTextColor(uint32 color)
{
    m_colors.text = color;

    return this;
}

void UI_Textbox::onClick()
{
    sUIManager->FocusElement(this);
}

const wchar_t* allowedCharacters = L" .,-_?!()/\\=+*;[]{}°@#$%^&<>"; /* ìšèøžýáíéùúóò */

bool isAllowedCharacter(wchar_t c)
{
    if ((c >= 'a' && c <= 'z') ||
        (c >= 'A' && c <= 'Z') ||
        (c >= '0' && c <= '9'))
        return true;

    for (uint32 i = 0; i < wcslen(allowedCharacters); i++)
        if (c == allowedCharacters[i])
            return true;

    return false;
}

void UI_Textbox::onKeyPress(uint16 key, bool press, bool printable)
{
    if (press)
    {
        if (printable)
        {
            if (isAllowedCharacter((wchar_t)key))
            {
                std::wstring nstr = m_text.c_str();
                nstr += (wchar_t)key;

                SetText(nstr.c_str());
            }
        }
        else
        {
            if (key == '\b')
            {
                SetText(m_text.substr(0,m_text.size()-1).c_str());
            }
        }
    }
}

/*UI_Textbox* UI_Textbox::SetOnClickCallback(void (*click)(UI_Element*))
{
    m_callbacks.onClick = click;

    return this;
}*/

void UI_Textbox::Draw()
{
    //if (m_border.width > 0)
    {
        //if (!m_border.plastic)
        {
            SF->Drawing->DrawRectangle(m_positionX-m_border.width, m_positionY-m_border.width, m_width+2*m_border.width+2*m_horizontalPadding, m_height+2*m_border.width+2*m_verticalPadding, m_border.color, 0);
            SF->Drawing->DrawRectangle(m_positionX, m_positionY, m_width+2*m_horizontalPadding, m_height+2*m_verticalPadding, m_colors.background, 0);
            glColor4ub(GET_COLOR_R(m_colors.text),GET_COLOR_G(m_colors.text),GET_COLOR_B(m_colors.text), 255);
            SF->Drawing->PrintText(m_cached.textFont, m_positionX+m_horizontalPadding+m_border.width, m_positionY+m_verticalPadding+(m_height/2)-(m_cached.textHeight/2), 0, WW_NO_WRAP, m_cached.printedString.c_str());
        }
        //else
        //{
            // TODO: plastic border
        //}
    }
}
