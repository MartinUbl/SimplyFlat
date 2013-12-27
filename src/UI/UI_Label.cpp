#include <SimplyFlat.h>
#include <UIManager.h>
#include <UI.h>

UI_Label::UI_Label(const char* name, int32 x, int32 y, const wchar_t* text, int32 font, uint32 textColor)
    : UI_Element(name, x, y, 0, 0)
{
    SetText(text);
    m_textFont = font;
    m_textColor = textColor;
}

UI_Label* UI_Label::SetText(const wchar_t* text)
{
    m_text = text;

    return this;
}

const wchar_t* UI_Label::GetText()
{
    return m_text.c_str();
}

UI_Label* UI_Label::SetTextColor(uint32 color)
{
    m_textColor = color;

    return this;
}

void UI_Label::Draw()
{
    SF->Drawing->PrintText(m_textFont, m_positionX, m_positionY, 0, WW_NO_WRAP, m_text.c_str());
}
