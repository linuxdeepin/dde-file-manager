// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEFAULTITEMMANAGER_H
#define DEFAULTITEMMANAGER_H

#include "dfmplugin_bookmark_global.h"

#include "bookmarkmanager.h"

#include <QObject>

namespace dfmplugin_bookmark {

class DefaultItemManagerPrivate;
class DefaultItemManager : public QObject
{
    Q_OBJECT
    friend class DefaultItemManagerPrivate;
    DefaultItemManagerPrivate *const d = nullptr;

public:
    static DefaultItemManager *instance();

    void initDefaultItems();
    void initPreDefineItems();

    QMap<QString, QUrl> defaultItemUrls();
    QMap<QString, QUrl> preDefItemUrls();
    QList<BookmarkData> defaultItemInitOrder();
    QList<BookmarkData> defaultPreDefInitOrder();

    bool isDefaultItem(const BookmarkData &data);
    bool isPreDefItem(const BookmarkData &data);

private:
    explicit DefaultItemManager(QObject *parent = nullptr);
};

}

#endif   // DEFAULTITEMMANAGER_H
