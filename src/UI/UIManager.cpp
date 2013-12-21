#include <SimplyFlat.h>
#include <UIManager.h>
#include <UI.h>

UIManager::UIManager()
{
    //
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

void UIManager::Draw()
{
    for (std::list<UI_Element*>::iterator itr = m_elements.begin(); itr != m_elements.end(); )
    {
        if ((*itr)->IsBeingDeleted())
        {
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

bool UIManager::MouseClick(uint32 x, uint32 y)
{
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
        }
    }
    return false;
}
