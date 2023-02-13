// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
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
    void addPluginItem(const QVariantMap &args);
    QMap<QString, QUrl> defaultItemUrls();
    QList<BookmarkData> defaultItemInitOrder();
    QMap<QString, QVariantMap> pluginItemData();
    bool isDefaultItem(const BookmarkData &data);
    bool isDefaultUrl(const BookmarkData &data);
    bool isDefaultPluginItem(const QString &name);
    BookmarkData pluginItemDataToBookmark(const QVariantMap &data);
Q_SIGNALS:
    void pluginItemDataAdded(const QUrl &url, const QString &bookmarkName, bool isDefaultItem, int index);

private:
    explicit DefaultItemManager(QObject *parent = nullptr);
};

}

#endif   // DEFAULTITEMMANAGER_H
