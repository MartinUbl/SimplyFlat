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
        void DeleteElement(UI_Element* elem); // TODO: assign "handle" ID to every element
        void Draw();

        bool MouseClick(uint32 x, uint32 y);

    private:
        std::list<UI_Element*> m_elements;
};

#define sUIManager SimplyFlatSingleton<UIManager>::instance()

#endif
