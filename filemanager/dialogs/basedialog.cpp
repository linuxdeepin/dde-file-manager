#include "basedialog.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QWindow>
#include <QShowEvent>
#include <QDebug>

BaseDialog::BaseDialog(QWidget *parent) : DWidget(parent)
{

}

BaseDialog::~BaseDialog()
{

}

void BaseDialog::adjustPosition(QWidget *w)
{
    QPoint p(0, 0);
    int extraw = 0, extrah = 0, scrn = 0;
    if (w)
       w = w->window();
    QRect desk;
    if (w) {
       scrn = QApplication::desktop()->screenNumber(w);
    } else if (QApplication::desktop()->isVirtualDesktop()) {
       scrn = QApplication::desktop()->screenNumber(QCursor::pos());
    } else {
       scrn = QApplication::desktop()->screenNumber(this);
    }
    desk = QApplication::desktop()->availableGeometry(scrn);

    QWidgetList list = QApplication::topLevelWidgets();
    for (int i = 0; (extraw == 0 || extrah == 0) && i < list.size(); ++i) {
       QWidget * current = list.at(i);
       if (current->isVisible()) {
           int framew = current->geometry().x() - current->x();
           int frameh = current->geometry().y() - current->y();

           extraw = qMax(extraw, framew);
           extrah = qMax(extrah, frameh);
       }
    }

    // sanity check for decoration frames. With embedding, we
    // might get extraordinary values
    if (extraw == 0 || extrah == 0 || extraw >= 10 || extrah >= 40) {
       extrah = 40;
       extraw = 10;
    }


    if (w) {
       // Use pos() if the widget is embedded into a native window
       QPoint pp;
       if (w->windowHandle() && w->windowHandle()->property("_q_embedded_native_parent_handle").value<WId>())
           pp = w->pos();
       else
           pp = w->mapToGlobal(QPoint(0,0));
       p = QPoint(pp.x() + w->width()/2,
                   pp.y() + w->height()/ 2);

    } else {
       // p = middle of the desktop
       p = QPoint(desk.x() + desk.width()/2, desk.y() + desk.height()/2);
    }

    // p = origin of this
    p = QPoint(p.x()-width()/2 - extraw,
               p.y()-height()/2 - extrah);


    if (p.x() + extraw + width() > desk.x() + desk.width())
       p.setX(desk.x() + desk.width() - width() - extraw);
    if (p.x() < desk.x())
       p.setX(desk.x());

    if (p.y() + extrah + height() > desk.y() + desk.height())
       p.setY(desk.y() + desk.height() - height() - extrah);
    if (p.y() < desk.y())
       p.setY(desk.y());

    move(p);
}

void BaseDialog::showEvent(QShowEvent *event)
{
    if (!event->spontaneous() && !testAttribute(Qt::WA_Moved)) {
        Qt::WindowStates  state = windowState();
        adjustPosition(parentWidget());
        setAttribute(Qt::WA_Moved, false); // not really an explicit position
        if (state != windowState())
            setWindowState(state);
    }
}

void BaseDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape){
        close();
    }
}
