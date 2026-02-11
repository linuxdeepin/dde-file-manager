// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COLLECTIONWIDGET_P_H
#define COLLECTIONWIDGET_P_H

#include "collectionwidget.h"

#include <QVBoxLayout>
#include <QTimer>
#include <QPointer>

namespace ddplugin_organizer {

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
    QPixmap freezePixmap;
    bool freeze = false;
    QTimer updateSnapshotTimer;

    QPointer<CollectionDataProvider> provider = nullptr;
};

}

#endif   // COLLECTIONWIDGET_P_H
