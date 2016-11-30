#include "dfilemenu.h"

DFileMenu::DFileMenu(QMenu *parent)
    : QMenu(parent)
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

QAction *DFileMenu::actionAt(int index) const
{
    return actions().value(index);
}

QAction *DFileMenu::actionAt(const QString &text) const
{
    for (QAction *action : actions()) {
        if (action->text() == text) {
            return action;
        }
    }

    return Q_NULLPTR;
}

QAction *DFileMenu::exec()
{
    return QMenu::exec(QCursor::pos());
}
