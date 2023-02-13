// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BACKGROUNDMANAGER_P_H
#define BACKGROUNDMANAGER_P_H

#include "backgroundmanager.h"
#include "backgroundservice.h"

#include <com_deepin_wm.h>

#include <QObject>

DDP_BACKGROUND_BEGIN_NAMESPACE

class BackgroundManagerPrivate : public QObject
{
    Q_OBJECT
public:
    explicit BackgroundManagerPrivate(BackgroundManager *qq);
    ~BackgroundManagerPrivate();
    bool isEnableBackground();

    inline QRect relativeGeometry(const QRect &geometry)
    {
        return QRect(QPoint(0, 0), geometry.size());
    }

public:
    BackgroundManager *const q = nullptr;
    BackgroundService *service = nullptr;
    QMap<QString, DFMBASE_NAMESPACE::BackgroundWidgetPointer> backgroundWidgets;
    QMap<QString, QString> backgroundPaths;
    bool enableBackground = true;
};

DDP_BACKGROUND_END_NAMESPACE

#endif // BACKGROUNDMANAGER_P_H
