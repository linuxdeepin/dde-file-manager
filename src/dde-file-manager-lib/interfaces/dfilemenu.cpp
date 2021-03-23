/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
