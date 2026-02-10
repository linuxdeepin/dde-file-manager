// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COLLECTIONDATAPROVIDER_H
#define COLLECTIONDATAPROVIDER_H

#include "ddplugin_organizer_global.h"
#include "organizer_defines.h"

#include <QObject>
#include <QHash>

namespace ddplugin_organizer {

class CollectionDataProvider : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(CollectionDataProvider)
public:
    explicit CollectionDataProvider(QObject *parent = nullptr);
    virtual ~CollectionDataProvider();
    virtual QString key(const QUrl &) const;
    virtual QString name(const QString &key) const;
    virtual QList<QString> keys() const;
    virtual QList<QUrl> items(const QString &key) const;
    virtual bool contains(const QString &key, const QUrl &url) const;
    virtual bool sorted(const QString &key, const QList<QUrl> &urls);
    virtual void moveUrls(const QList<QUrl> &urls, const QString &targetKey, int targetIndex);
    virtual void addPreItems(const QString &targetKey, const QList<QUrl> &urls, int targetIndex);
    virtual bool checkPreItem(const QUrl &url, QString &key, int &index);
    virtual bool takePreItem(const QUrl &url, QString &key, int &index);
    virtual QString replace(const QUrl &oldUrl, const QUrl &newUrl) = 0;
    virtual QString append(const QUrl &) = 0;
    virtual QString prepend(const QUrl &) = 0;
    virtual void insert(const QUrl &, const QString &, const int) = 0;
    virtual QString remove(const QUrl &) = 0;
    virtual QString change(const QUrl &) = 0;
signals:
    void nameChanged(const QString &key, const QString &name);
    void itemsChanged(const QString &key);

protected:
    QHash<QString, CollectionBaseDataPtr> collections;
    QHash<QString, QPair<int, QList<QUrl>>> preCollectionItems;
};

}

#endif   // COLLECTIONDATAPROVIDER_H
