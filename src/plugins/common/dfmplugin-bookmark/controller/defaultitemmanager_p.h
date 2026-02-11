// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEFAULTITEMMANAGER_P_H
#define DEFAULTITEMMANAGER_P_H

#include "dfmplugin_bookmark_global.h"
#include "bookmarkmanager.h"

#include <QObject>

DPBOOKMARK_BEGIN_NAMESPACE

class DefaultItemManager;
class DefaultItemManagerPrivate : public QObject
{
    Q_OBJECT
    friend class DefaultItemManager;
    DefaultItemManager *const q;

    explicit DefaultItemManagerPrivate(DefaultItemManager *qq);

    QMap<QString, QUrl> defaultItemUrls;
    QMap<QString, QUrl> defaultPreDefItemUrls;
    QList<BookmarkData> defaultItemInitOrder;
    QList<BookmarkData> defaultItemPreDefOrder;
};

DPBOOKMARK_END_NAMESPACE

#endif   // SIDEBARVIEW_P_H
