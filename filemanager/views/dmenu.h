#ifndef DMENU_H
#define DMENU_H

#include <QMenu>

class DMenu : public QMenu
{
public:
    DMenu();
protected:
    void paintEvent(QPaintEvent *e);
};

#endif // DMENU_H
