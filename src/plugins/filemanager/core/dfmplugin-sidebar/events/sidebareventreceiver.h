/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#ifndef SIDEBAREVENTRECEIVER_H
#define SIDEBAREVENTRECEIVER_H

#include "dfmplugin_sidebar_global.h"

#include <QObject>

DPSIDEBAR_BEGIN_NAMESPACE

class SideBarEventReceiver : public QObject
{
    Q_OBJECT

public:
    static SideBarEventReceiver *instance();
    void bindEvents();

public slots:
    void handleSetContextMenuEnable(bool enable);
    QList<QUrl> handleGetGroupItems(quint64 winId, const QString &group);

    bool handleItemAdd(const QUrl &url, const QVariantMap &properties);
    bool handleItemRemove(const QUrl &url);
    bool handleItemUpdate(const QUrl &url, const QVariantMap &properties);
    bool handleItemInsert(int index, const QUrl &url, const QVariantMap &properties);
    void handleItemHidden(const QUrl &url, bool visible);   // TODO(zhangs): remove
    void handleItemTriggerEdit(quint64 winId, const QUrl &url);

private:
    explicit SideBarEventReceiver(QObject *parent = nullptr);
};

DPSIDEBAR_END_NAMESPACE

#endif   // SIDEBAREVENTRECEIVER_H
