#ifndef DFILEMENU_H
#define DFILEMENU_H

#include <DMenu>

DWIDGET_USE_NAMESPACE

class DFileMenu : public DMenu
{
    Q_OBJECT

public:
    explicit DFileMenu(DMenu * parent = 0);
};

#endif // DFILEMENU_H
