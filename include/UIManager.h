#ifndef SF_UIMANAGER_H
#define SF_UIMANAGER_H

class UI_Element;

class UIManager
{
    public:
        UIManager();
        ~UIManager();

        void Init();

        void AddElement(UI_Element* elem);
        void DeleteElement(UI_Element* elem, bool force = false); // TODO: assign "handle" ID to every element
        void ClearElements(bool force = false);
        void Draw();

        UI_Element* GetElementByName(const char* name);

        void FocusElement(UI_Element* elem);

        bool MouseClick(uint32 x, uint32 y);
        void MouseMove(uint32 x, uint32 y);
        bool KeyPress(uint16 key, bool press, bool printable);

    private:
        std::list<UI_Element*> m_elements;
        UI_Element* m_focusElement;
};

#define sUIManager SimplyFlatSingleton<UIManager>::instance()

#endif
