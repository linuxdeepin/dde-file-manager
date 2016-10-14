#ifndef DFILEMENU_H
#define DFILEMENU_H

#include <DMenu>
#include "fmevent.h"

DWIDGET_USE_NAMESPACE

class DFileMenu : public DMenu
{
    Q_OBJECT

public:
    explicit DFileMenu(DMenu * parent = 0);
    FMEvent event() const;
    void setEvent(const FMEvent &event);

private:
    FMEvent m_event;
};

#endif // DFILEMENU_H
