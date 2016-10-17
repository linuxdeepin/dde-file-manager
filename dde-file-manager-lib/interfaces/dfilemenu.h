#ifndef DFILEMENU_H
#define DFILEMENU_H

#include <DMenu>
#include "dfmevent.h"

DWIDGET_USE_NAMESPACE

class DFileMenu : public DMenu
{
    Q_OBJECT

public:
    explicit DFileMenu(DMenu * parent = 0);
    DFMEvent event() const;
    void setEvent(const DFMEvent &event);

private:
    DFMEvent m_event;
};

#endif // DFILEMENU_H
