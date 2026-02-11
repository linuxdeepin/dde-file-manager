// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CUSTOMDATAHANDLER_H
#define CUSTOMDATAHANDLER_H

#include "models/modeldatahandler.h"
#include "organizer_defines.h"
#include "mode/collectiondataprovider.h"

#include <QUrl>
#include <QHash>

namespace ddplugin_organizer {

class CustomDataHandler : public CollectionDataProvider, public ModelDataHandler
{
    Q_OBJECT
public:
    explicit CustomDataHandler(QObject *parent = nullptr);
    ~CustomDataHandler() override;
    void check(const QSet<QUrl> &vaild);
    QList<CollectionBaseDataPtr> baseDatas() const;
    bool addBaseData(const CollectionBaseDataPtr &base);
    void removeBaseData(const QString &key);
public:
    virtual bool reset(const QList<CollectionBaseDataPtr> &);
public:
    QString replace(const QUrl &oldUrl, const QUrl &newUrl) override;
    QString append(const QUrl &) override;
    QString prepend(const QUrl &) override;
    void insert(const QUrl &url, const QString &key, const int index) override;
    QString remove(const QUrl &url) override;
    QString change(const QUrl &) override;
public:
    bool acceptInsert(const QUrl &url) override;
    QList<QUrl> acceptReset(const QList<QUrl> &urls) override;
    bool acceptRename(const QUrl &oldUrl, const QUrl &newUrl) override;
};

}

#endif // CUSTOMDATAHANDLER_H
