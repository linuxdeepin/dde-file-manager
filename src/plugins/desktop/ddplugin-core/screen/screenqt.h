// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SCREENQT_H
#define SCREENQT_H

#include "ddplugin_core_global.h"

#include <dfm-base/interfaces/screen/abstractscreen.h>

#include <QScreen>

DDPCORE_BEGIN_NAMESPACE

class ScreenQt : public DFMBASE_NAMESPACE::AbstractScreen
{
    Q_OBJECT
public:
    explicit ScreenQt(QScreen *screen, QObject *parent = nullptr);
    virtual QString name() const override;
    virtual QRect geometry() const override;
    virtual QRect availableGeometry() const override;
    virtual QRect handleGeometry() const override;
    QScreen *screen() const;

private:
    bool checkAvailableGeometry(const QRect &ava, const QRect &scr) const;
    QScreen *qscreen = nullptr;
};

DDPCORE_END_NAMESPACE

#endif   // SCREENQT
