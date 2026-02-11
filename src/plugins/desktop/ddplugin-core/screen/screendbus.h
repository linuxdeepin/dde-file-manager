// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SCREENDBUS_H
#define SCREENDBUS_H

#include "ddplugin_core_global.h"

#include <interfaces/screen/abstractscreen.h>

#include <QMutex>

class DBusMonitor;

DDPCORE_BEGIN_NAMESPACE

class ScreenDBus : public DFMBASE_NAMESPACE::AbstractScreen
{
    Q_OBJECT
public:
    explicit ScreenDBus(DBusMonitor *monitor, QObject *parent = nullptr);
    virtual ~ScreenDBus() override;
    virtual QString name() const override;
    virtual QRect geometry() const override;
    virtual QRect availableGeometry() const override;
    QRect handleGeometry() const override;
    QString path() const;
    bool enabled() const;
private:
    DBusMonitor *dbusMonitor = nullptr;
};

DDPCORE_END_NAMESPACE
#endif // SCREENDBUS_H
