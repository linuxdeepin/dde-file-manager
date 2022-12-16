/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangtingwei<wangtingwei@uniontech.com>
 *
 * Maintainer: wangtingwei<wangtingwei@uniontech.com>
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
#ifndef BACKGROUNDDDE_H
#define BACKGROUNDDDE_H

#include "ddplugin_background_global.h"
#include "backgroundservice.h"
#include "appearance_interface.h"

#include "base/configs/dconfig/dconfigmanager.h"

#include <DWindowManagerHelper>

#include <QObject>

DCORE_BEGIN_NAMESPACE
class DConfig;
DCORE_END_NAMESPACE

DDP_BACKGROUND_BEGIN_NAMESPACE

using InterFace = org::deepin::dde::Appearance1;

class BackgroundDDE : public BackgroundService
{
    Q_OBJECT
public:
    explicit BackgroundDDE(QObject *parent = nullptr);
    ~BackgroundDDE() override;
public:
    QString background(const QString &screen) override;
    QString getDefaultBackground() override;
protected:
    QString getBackgroundFromDDE(const QString &screen);
    QString getBackgroundFromConfig(const QString &screen);
private slots:
    void onAppearanceValueChanged(const QString& key);
protected:
    InterFace *interface = nullptr;
    DTK_CORE_NAMESPACE::DConfig *apperanceConf = nullptr;
};

DDP_BACKGROUND_END_NAMESPACE

#endif // BACKGROUNDDDE_H
