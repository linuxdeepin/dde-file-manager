// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GSETTINGMANAGER_P_H
#define GSETTINGMANAGER_P_H

#include <dfm-base/dfm_base_global.h>

#include <QMap>
#include <QReadWriteLock>

class QGSettings;

DFMBASE_BEGIN_NAMESPACE

class GSettingManager;
class GSettingManagerPrivate
{
    friend class GSettingManager;
    GSettingManager *q { nullptr };
    QMap<QString, QGSettings *> settings;
    QReadWriteLock lock;

public:
    GSettingManagerPrivate(GSettingManager *qq)
        : q(qq) {}
};

DFMBASE_END_NAMESPACE

#endif   // GSETTINGMANAGER_P_H
