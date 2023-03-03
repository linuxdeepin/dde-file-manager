// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ORGANIZER_DEFINES_H
#define ORGANIZER_DEFINES_H

#include "ddplugin_organizer_global.h"

#include <QString>
#include <QUrl>
#include <QSharedPointer>
#include <QRect>

namespace ddplugin_organizer {

enum OrganizerMode {
    kNormalized = 0,
    kCustom
};

enum Classifier {
    kType = 0,
    kTimeCreated,
    kTimeModified,
    kLabel,
    kName,
    kSize
};

enum ItemCategory {
    kCatNone = 0,
    kCatApplication = 0x01,
    kCatDocument = 0x02,
    kCatPicture = 0x04,
    kCatVideo = 0x08,
    kCatMusic = 0x10,
    kCatFloder = 0x20,

    kCatEnd = kCatFloder,
    kCatAll = kCatApplication | kCatDocument | kCatPicture | kCatVideo | kCatMusic | kCatFloder,
    kCatDefault = -1
};
Q_DECLARE_FLAGS(ItemCategories, ItemCategory)

enum DisplaySize {
    kSmaller = 0,
    kNormal,
    kLarger
};

enum CollectionFrameSize {
    kSmall = 0,
    kLarge
};

class CollectionBaseData
{
public:
    QString name;
    QString key;
    QList<QUrl> items;
};

typedef QSharedPointer<CollectionBaseData> CollectionBaseDataPtr;

struct CollectionStyle
{
    int screenIndex = -1;
    QString key;
    QRect rect;
    CollectionFrameSize sizeMode = CollectionFrameSize::kSmall;
};
}

Q_DECLARE_METATYPE(ddplugin_organizer::CollectionFrameSize);

#endif // ORGANIZER_DEFINES_H
