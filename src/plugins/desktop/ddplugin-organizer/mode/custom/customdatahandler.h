/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#ifndef CUSTOMDATAHANDLER_H
#define CUSTOMDATAHANDLER_H

#include "models/modeldatahandler.h"
#include "organizer_defines.h"
#include "mode/collectiondataprovider.h"

#include <QUrl>
#include <QHash>

DDP_ORGANIZER_BEGIN_NAMESPACE

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
    void insert(const QUrl &url, const QString &key, const int index) override;
    QString remove(const QUrl &url) override;
    QString change(const QUrl &) override;
public:
    bool acceptInsert(const QUrl &url) override;
    QList<QUrl> acceptReset(const QList<QUrl> &urls) override;
    bool acceptRename(const QUrl &oldUrl, const QUrl &newUrl) override;
};

DDP_ORGANIZER_END_NAMESPACE

#endif // CUSTOMDATAHANDLER_H
