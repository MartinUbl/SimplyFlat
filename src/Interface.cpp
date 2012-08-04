#include <Defines.h>
#include <SimplyFlat.h>

SimplyFlat::t_Interface::t_Interface()
{
    for (uint32 i = 0; i < KEY_COUNT; i++)
    {
        keys[i] = false;
        handlers[i] = NULL;
    }
}

SimplyFlat::t_Interface::~t_Interface()
{
}

void SimplyFlat::t_Interface::HookEvent(uint16 key, void (*handler)(bool))
{
    if (handler == NULL)
        return;

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

        if (handlers[key] != NULL)
            (handlers[key])(press);
    }
}
