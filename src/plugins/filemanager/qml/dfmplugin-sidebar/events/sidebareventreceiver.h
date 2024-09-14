// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
    void handleSidebarUpdateSelection(quint64 winId);

private:
    explicit SideBarEventReceiver(QObject *parent = nullptr);
};

DPSIDEBAR_END_NAMESPACE

#endif   // SIDEBAREVENTRECEIVER_H
