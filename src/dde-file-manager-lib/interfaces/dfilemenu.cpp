// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfilemenu.h"
#include "dfmevent.h"
#include "accessibility/acintelfunctions.h"
#include <QTimer>
#include <QApplication>

DFileMenu::DFileMenu(QWidget *parent)
    : QMenu(parent)
{
    this->setAccessibleInfo(AC_FILE_MENU_DEFAULT);
}

const QSharedPointer<DFMMenuActionEvent> DFileMenu::makeEvent(DFMGlobal::MenuAction action) const
{
    DFMMenuActionEvent *event = new DFMMenuActionEvent(m_sender, this, m_currentUrl, m_selectedUrls, action, m_clickedIndex);

    event->setWindowId(m_eventId);

    return QSharedPointer<DFMMenuActionEvent>(event);
}

void DFileMenu::setEventData(const DUrl &currentUrl, const DUrlList &selectedUrls, quint64 eventId, const QObject *sender, const QModelIndex &index)
{
    m_currentUrl = currentUrl;
    m_selectedUrls = selectedUrls;
    m_eventId = eventId;
    m_sender = sender;
    //设置当前点击的index
    m_clickedIndex = index;
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

QAction *DFileMenu::exec(const QPoint &pos, QAction *at)
{
    return QMenu::exec(pos,at);
}

quint64 DFileMenu::eventId() const
{
    return m_eventId;
}

QSet<DFMGlobal::MenuAction> DFileMenu::ignoreMenuActions() const
{
    return m_ignoreMenuActions;
}

void DFileMenu::setIgnoreMenuActions(const QSet<DFMGlobal::MenuAction> &ignoreMenuActions)
{
    m_ignoreMenuActions = ignoreMenuActions;
}

DUrlList DFileMenu::selectedUrls() const
{
    return m_selectedUrls;
}

// 重定义，防止多次右键点击造成崩溃
void DFileMenu::deleteLater(QWidget *w)
{
    if (w)
        qApp->setActiveWindow(w);
    QMenu::deleteLater();
}

void DFileMenu::mouseMoveEvent(QMouseEvent * event)
{
    QMenu::mouseMoveEvent(event);
    update();
}

void DFileMenu::setAccessibleInfo(const QString& name)
{
    AC_SET_OBJECT_NAME(this,name);
    AC_SET_ACCESSIBLE_NAME(this,name);
}
