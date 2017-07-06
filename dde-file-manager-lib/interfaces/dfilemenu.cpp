#include "dfilemenu.h"
#include "dfmevent.h"

DFileMenu::DFileMenu(QWidget *parent)
    : QMenu(parent)
{

}

const QSharedPointer<DFMMenuActionEvent> DFileMenu::makeEvent(DFMGlobal::MenuAction action) const
{
    DFMMenuActionEvent *event = new DFMMenuActionEvent(m_sender, this, m_currentUrl, m_selectedUrls, action);

    event->setWindowId(m_eventId);

    return QSharedPointer<DFMMenuActionEvent>(event);
}

void DFileMenu::setEventData(const DUrl &currentUrl, const DUrlList &selectedUrls, quint64 eventId, const QObject *sender)
{
    m_currentUrl = currentUrl;
    m_selectedUrls = selectedUrls;
    m_eventId = eventId;
    m_sender = sender;
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

quint64 DFileMenu::eventId() const
{
    return m_eventId;
}
