#include "dfilemenu.h"

DFileMenu::DFileMenu(DMenu *parent)
    : DMenu(parent)
{

}


DFMEvent DFileMenu::event() const
{
    return m_event;
}

void DFileMenu::setEvent(const DFMEvent &event)
{
    m_event = event;
}
