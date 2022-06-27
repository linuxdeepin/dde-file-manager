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
#ifndef COLLECTIONHOLDER_P_H
#define COLLECTIONHOLDER_P_H

#include "collection/collectionholder.h"
#include "mode/collectiondataprovider.h"
#include "private/surface.h"
#include "organizer_defines.h"

#include <QPointer>
#include <QTimer>

DDP_ORGANIZER_BEGIN_NAMESPACE

class CollectionHolderPrivate : public QObject
{
    Q_OBJECT
public:
    explicit CollectionHolderPrivate(const QString &uuid, CollectionDataProvider *dataProvider, CollectionHolder *qq, QObject *parent = nullptr);
    ~CollectionHolderPrivate();

public slots:
    void onAdjustFrameSize(const CollectionFrameSize &size);

public:
    CollectionHolder *q;
    QString id;
    QPointer<CollectionDataProvider> provider = nullptr;
    QPointer<FileProxyModel> model = nullptr;
    QPointer<CollectionFrame> frame = nullptr;
    QPointer<CollectionWidget> widget = nullptr;
    QPointer<Surface> surface = nullptr;
    QTimer styleTimer;
};

DDP_ORGANIZER_END_NAMESPACE

#endif // COLLECTIONHOLDER_P_H
