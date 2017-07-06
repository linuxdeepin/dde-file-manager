#ifndef DFILEMENU_H
#define DFILEMENU_H

#include <QMenu>

#include "dfmglobal.h"
#include "durl.h"

class DFMMenuActionEvent;
class DFileMenu : public QMenu
{
    Q_OBJECT

public:
    explicit DFileMenu(QWidget * parent = 0);

    const QSharedPointer<DFMMenuActionEvent> makeEvent(DFMGlobal::MenuAction action) const;
    void setEventData(const DUrl &currentUrl, const DUrlList &selectedUrls, quint64 eventId = 0, const QObject *sender = 0);

    QAction *actionAt(int index) const;
    QAction *actionAt(const QString &text) const;

    QAction *exec();
    using QMenu::exec;

    quint64 eventId() const;

private:
    DUrl m_currentUrl;
    DUrlList m_selectedUrls;
    quint64 m_eventId = 0;
    const QObject *m_sender = Q_NULLPTR;
};

#endif // DFILEMENU_H
