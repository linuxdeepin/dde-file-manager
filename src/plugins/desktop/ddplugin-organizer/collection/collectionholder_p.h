// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COLLECTIONHOLDER_P_H
#define COLLECTIONHOLDER_P_H

#include "collection/collectionholder.h"
#include "mode/collectiondataprovider.h"
#include "private/surface.h"
#include "organizer_defines.h"

#include <QPointer>
#include <QTimer>

namespace ddplugin_organizer {

class CollectionHolderPrivate : public QObject
{
    Q_OBJECT
public:
    explicit CollectionHolderPrivate(const QString &uuid, CollectionDataProvider *dataProvider, CollectionHolder *qq, QObject *parent = nullptr);
    ~CollectionHolderPrivate();

public slots:
    void onAdjustFrameSizeMode(const CollectionFrameSize &size);

public:
    CollectionHolder *q;
    QString id;
    int screenIndex = 1;
    CollectionFrameSize sizeMode = CollectionFrameSize::kMiddle;
    QPointer<CollectionDataProvider> provider = nullptr;
    QPointer<CollectionModel> model = nullptr;
    QPointer<CollectionFrame> frame = nullptr;
    QPointer<CollectionWidget> widget = nullptr;
    QPointer<Surface> surface = nullptr;
    QTimer styleTimer;
    bool customGeo = false;
};

}

#endif   // COLLECTIONHOLDER_P_H
