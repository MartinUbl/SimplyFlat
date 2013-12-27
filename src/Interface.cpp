#include <Defines.h>
#include <SimplyFlat.h>
#include <UIManager.h>

SimplyFlat::t_Interface::t_Interface()
{
    for (uint32 i = 0; i < KEY_COUNT; i++)
    {
        keys[i] = false;
        handlers[i] = NULL;
    }

    for (uint32 i = 0; i < MOUSE_BUTTON_MAX; i++)
        mousebuttons[i] = false;

    mousehandler = NULL;
    allKeyHandler = NULL;
}

SimplyFlat::t_Interface::~t_Interface()
{
}

void SimplyFlat::t_Interface::HookEvent(uint16 key, void (*handler)(uint16,bool))
{
    if (handler == NULL)
        return;

    if (key == 0)
    {
        allKeyHandler = handler;
        return;
    }

    if (key > KEY_COUNT)
        return;

    handlers[key] = handler;
}

void SimplyFlat::t_Interface::KeyEvent(uint16 key, bool press)
{
    if (key > KEY_COUNT)
        return;

    if (keys[key] != press)
    {
        keys[key] = press;

        if (sUIManager->KeyPress(key, press, false))
            return;

        if (allKeyHandler != NULL)
            (allKeyHandler)(key,press);
        if (handlers[key] != NULL)
            (handlers[key])(key,press);
    }
}

void SimplyFlat::t_Interface::PrintableCharEvent(uint16 key)
{
    sUIManager->KeyPress(key, true, true);
}

void SimplyFlat::t_Interface::MouseEvent(bool left, bool press)
{
    mousebuttons[left?MOUSE_BUTTON_LEFT:MOUSE_BUTTON_RIGHT] = press;

    // UI manager has the privileged access to events
    if (sUIManager->MouseClick(GetMouseX(), GetMouseY()))
        return;

    if (mousehandler)
        mousehandler(left,press);
}

void SimplyFlat::t_Interface::HookMouseEvent(void (*handler)(bool, bool))
{
    mousehandler = handler;
}

void SimplyFlat::t_Interface::SetMouseXY(uint32 x, uint32 y)
{
    mouseXY[0] = x;
    mouseXY[1] = y;
}

uint32 SimplyFlat::t_Interface::GetMouseX()
{
    return mouseXY[0];
}

uint32 SimplyFlat::t_Interface::GetMouseY()
{
    return mouseXY[1];
}
