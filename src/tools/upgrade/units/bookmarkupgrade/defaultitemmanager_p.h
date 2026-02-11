// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEFAULTITEMMANAGER_P_H
#define DEFAULTITEMMANAGER_P_H

#include "units/bookmarkupgradeunit.h"
#include <QObject>
#include <QMap>
#include <QList>

namespace dfm_upgrade {
class DefaultItemManager;
class DefaultItemManagerPrivate : public QObject
{
    Q_OBJECT
    friend class DefaultItemManager;
    DefaultItemManager *const q;

    explicit DefaultItemManagerPrivate(DefaultItemManager *qq);

    QList<BookmarkData> defaultItemInitOrder;
    QList<BookmarkData> defaultItemPreDefOrder;

    QMap<QString, QVariantMap> pluginItemData;
    QStringList defaultPluginItem;
};

}

#endif   // SIDEBARVIEW_P_H
