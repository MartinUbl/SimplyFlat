#include <SimplyFlat.h>
#include <UIManager.h>
#include <UI.h>

/**
 * Base class UI_Element
 */

UI_Element::UI_Element(const char* name, int32 x, int32 y, uint32 width, uint32 height)
{
    m_toDelete = false;

    m_positionX = x;
    m_positionY = y;

    m_width = width;
    m_height = height;

    SetName(name);

    sUIManager->AddElement(this);
    // TODO: create event
}

void UI_Element::Delete()
{
    m_toDelete = true;
}

bool UI_Element::IsBeingDeleted()
{
    return m_toDelete;
}
