#include "dmenu.h"
#include <QPainter>

DMenu::DMenu()
{
setAttribute(Qt::WA_TranslucentBackground, true);
}

void DMenu::paintEvent(QPaintEvent *e)
{
//    QPainter p(this);
//    p.drawRoundedRect(0, 0, width() - 1, height() - 1, 20, 20);
    QMenu::paintEvent(e);
}
