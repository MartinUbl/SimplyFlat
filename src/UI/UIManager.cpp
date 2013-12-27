#include <SimplyFlat.h>
#include <UIManager.h>
#include <UI.h>

UIManager::UIManager()
{
    m_focusElement = NULL;
}

UIManager::~UIManager()
{
}

void UIManager::Init()
{
    //
}

void UIManager::AddElement(UI_Element* elem)
{
    m_elements.push_back(elem);
}

void UIManager::DeleteElement(UI_Element* elem, bool force)
{
    if (!elem)
        return;

    if (elem == m_focusElement)
        m_focusElement = NULL;

    for (std::list<UI_Element*>::iterator itr = m_elements.begin(); itr != m_elements.end(); ++itr)
    {
        if (*itr == elem)
        {
            if (force)
                m_elements.erase(itr);
            else
                (*itr)->Delete();
            break;
        }
    }
}

void UIManager::ClearElements(bool force)
{
    m_focusElement = NULL;

    if (force)
    {
        for (std::list<UI_Element*>::iterator itr = m_elements.begin(); itr != m_elements.end(); )
        {
            delete (*itr);
            itr = m_elements.erase(itr);
        }
    }
    else
    {
        for (std::list<UI_Element*>::iterator itr = m_elements.begin(); itr != m_elements.end(); )
        {
            (*itr)->Delete();
        }
    }
}

void UIManager::Draw()
{
    for (std::list<UI_Element*>::iterator itr = m_elements.begin(); itr != m_elements.end(); )
    {
        if ((*itr)->IsBeingDeleted())
        {
            if (*itr == m_focusElement)
                m_focusElement = NULL;

            delete (*itr);
            itr = m_elements.erase(itr);

            continue;
        }
        else
        {
            (*itr)->Draw();
            ++itr;

            continue;
        }
    }
}

bool compare_strings(const char* first, const char* second)
{
    uint32 i = 0;
    while (true)
    {
        if (first[i] != second[i])
            return false;

        if (first[i] == '\0')
            return true;
    }
}

UI_Element* UIManager::GetElementByName(const char* name)
{
    for (std::list<UI_Element*>::iterator itr = m_elements.begin(); itr != m_elements.end(); ++itr)
    {
        if (compare_strings(name, (*itr)->GetName()))
            return *itr;
    }

    return NULL;
}

bool UIManager::MouseClick(uint32 x, uint32 y)
{
    bool elemTaken = false;
    UI_Element* elem;
    for (std::list<UI_Element*>::iterator itr = m_elements.begin(); itr != m_elements.end(); ++itr)
    {
        if ((*itr)->IsBeingDeleted())
            continue;

        elem = *itr;

        if (x > elem->GetPositionX() && x < elem->GetPositionX()+elem->GetWidth() &&
            y > elem->GetPositionY() && y < elem->GetPositionY()+elem->GetHeight())
        {
            elem->onClick();
            elemTaken = true;
        }
    }
    return elemTaken;
}

void UIManager::FocusElement(UI_Element* elem)
{
    if (m_focusElement)
        m_focusElement->onFocusLost();

    m_focusElement = elem;
    m_focusElement->onFocus();
}

void UIManager::MouseMove(uint32 x, uint32 y)
{
    // TODO
}

bool UIManager::KeyPress(uint16 key, bool press, bool printable)
{
    if (m_focusElement)
        m_focusElement->onKeyPress(key, press, printable);
     return false;
}
