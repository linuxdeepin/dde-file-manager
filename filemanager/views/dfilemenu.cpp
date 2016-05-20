#include "dfilemenu.h"

DFileMenu::DFileMenu(DMenu *parent)
    : DMenu(parent)
{

}


FMEvent DFileMenu::event() const
{
    return m_event;
}

void DFileMenu::setEvent(const FMEvent &event)
{
    m_event = event;
}
