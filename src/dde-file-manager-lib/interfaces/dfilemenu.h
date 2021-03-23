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

#ifndef DFILEMENU_H
#define DFILEMENU_H

#include <QMenu>

#include "dfmglobal.h"
#include "durl.h"
#include "accessibility/ac-lib-file-manager.h"
#include <QModelIndex>

class DFMMenuActionEvent;
class DFileMenu : public QMenu
{
    Q_OBJECT

public:
    explicit DFileMenu(QWidget * parent = nullptr);
    ~DFileMenu() override { }

    const QSharedPointer<DFMMenuActionEvent> makeEvent(DFMGlobal::MenuAction action) const;
    void setEventData(const DUrl &currentUrl, const DUrlList &selectedUrls, quint64 eventId = 0, const QObject *sender = nullptr, const QModelIndex &index = QModelIndex());

    QAction *actionAt(int index) const;
    QAction *actionAt(const QString &text) const;

    QAction *exec();
    QAction *exec(const QPoint &pos, QAction *at = nullptr);
    using QMenu::exec;

    quint64 eventId() const;

    QSet<DFMGlobal::MenuAction> ignoreMenuActions() const;
    void setIgnoreMenuActions(const QSet<DFMGlobal::MenuAction> &ignoreMenuActions);

    DUrlList selectedUrls() const;
    void setCanUse(const bool canuse);
    void mouseMoveEvent(QMouseEvent * event) override;

    void setAccessibleInfo(const QString& name);

public Q_SLOTS:
    void deleteLater(QWidget *w = nullptr);

private:
    DUrl m_currentUrl;
    DUrlList m_selectedUrls;
    quint64 m_eventId = 0;
    const QObject *m_sender = Q_NULLPTR;
    QSet<DFMGlobal::MenuAction> m_ignoreMenuActions; //Don't handle MenuAction list in libdde-file-manager in m_ignoreMenuActions;
    QModelIndex m_clickedIndex;  //右键点击的index
};

#endif // DFILEMENU_H
