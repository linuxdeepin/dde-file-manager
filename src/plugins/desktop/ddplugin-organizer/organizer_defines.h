// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
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

inline const int kCollectionStretchThreshold = 10;
inline constexpr char kCollectionPropertyEditing[] { "collection_editing" };

enum OrganizerMode {
    kNormalized = 0,
    kCustom
};

enum OrganizeAction {
    kOnTrigger,
    kAlways
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
    kCatFolder = 0x20,
    kCatOther = 0x40,

    kCatEnd = kCatOther,
    kCatAll = kCatApplication | kCatDocument | kCatPicture | kCatVideo | kCatMusic | kCatFolder | kCatOther,
    kCatDefault = -1
};
Q_DECLARE_FLAGS(ItemCategories, ItemCategory)

inline const char kTypeKeyApp[] = "Type_Apps";
inline const char kTypeKeyDoc[] = "Type_Documents";
inline const char kTypeKeyPic[] = "Type_Pictures";
inline const char kTypeKeyVid[] = "Type_Videos";
inline const char kTypeKeyMuz[] = "Type_Music";
inline const char kTypeKeyFld[] = "Type_Folders";
inline const char kTypeKeyOth[] = "Type_Other";
inline const QMap<ItemCategory, QString> kCategory2Key {
    { kCatApplication, kTypeKeyApp },
    { kCatDocument, kTypeKeyDoc },
    { kCatPicture, kTypeKeyPic },
    { kCatVideo, kTypeKeyVid },
    { kCatMusic, kTypeKeyMuz },
    { kCatFolder, kTypeKeyFld },
    { kCatOther, kTypeKeyOth }
};

enum CollectionFrameSize {
    kMiddle = 0,
    kLarge,
    kSmall,
    kFree
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
    bool customGeo = false;
};
}

Q_DECLARE_METATYPE(ddplugin_organizer::CollectionFrameSize);

#endif   // ORGANIZER_DEFINES_H
