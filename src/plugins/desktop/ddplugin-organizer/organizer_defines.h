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
#ifndef ORGANIZER_DEFINES_H
#define ORGANIZER_DEFINES_H

#include "ddplugin_organizer_global.h"

#include <QString>
#include <QUrl>
#include <QSharedPointer>
#include <QRect>

DDP_ORGANIZER_BEGIN_NAMESPACE

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

enum class CollectionFrameSize {
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

class CollectionStyle
{
public:
    int screenIndex = -1;
    QString key;
    QRect rect;
};

DDP_ORGANIZER_END_NAMESPACE

#endif // ORGANIZER_DEFINES_H
