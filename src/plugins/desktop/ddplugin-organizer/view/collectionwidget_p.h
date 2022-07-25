/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
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
#ifndef COLLECTIONWIDGET_P_H
#define COLLECTIONWIDGET_P_H

#include "collectionwidget.h"

#include <QVBoxLayout>
#include <QPointer>

DDP_ORGANIZER_BEGIN_NAMESPACE

class CollectionDataProvider;

class CollectionWidgetPrivate : public QObject
{
    Q_OBJECT
public:
    explicit CollectionWidgetPrivate(const QString &uuid, CollectionDataProvider *dataProvider, CollectionWidget *qq, QObject *parent = nullptr);
    ~CollectionWidgetPrivate();

public slots:
    void onNameChanged(const QString &key, const QString &name);

public:
    CollectionWidget *q = nullptr;
    QString id;
    CollectionTitleBar *titleBar = nullptr;
    CollectionView *view = nullptr;
    QVBoxLayout *mainLayout = nullptr;

    QPointer<CollectionDataProvider> provider = nullptr;
};

DDP_ORGANIZER_END_NAMESPACE

#endif // COLLECTIONWIDGET_P_H
