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
#ifndef NORMALIZEDMODE_P_H
#define NORMALIZEDMODE_P_H

#include "mode/normalizedmode.h"
#include "fileclassifier.h"
#include "collection/collectionholder.h"

DDP_ORGANIZER_BEGIN_NAMESPACE

class NormalizedModePrivate : public QObject
{
    Q_OBJECT
public:
    explicit NormalizedModePrivate(NormalizedMode *qq);
    ~NormalizedModePrivate();

    QPoint findValidPos(QPoint &nextPos, int &currentIndex, CollectionStyle &style, const int width, const int height);

    void collectionStyleChanged(const QString &id);
public:
    void restore(const QList<CollectionBaseDataPtr> &cfgs);
    FileClassifier *classifier = nullptr;
    QHash<QString, CollectionHolderPointer> holders;
private:
    NormalizedMode *q;
};

DDP_ORGANIZER_END_NAMESPACE

#endif // NORMALIZEDMODE_P_H
