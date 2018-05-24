/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     zccrs <zccrs@live.com>
 *
 * Maintainer: zccrs <zhangjide@deepin.com>
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
#include "dfmapplication.h"
#include "private/dfmapplication_p.h"

#include "dfmsettings.h"

#include <QCoreApplication>

DFM_BEGIN_NAMESPACE

Q_GLOBAL_STATIC_WITH_ARGS(DFMSettings, gsGlobal, ("deepin/dde-file-manager", DFMSettings::GenericConfig))
Q_GLOBAL_STATIC_WITH_ARGS(DFMSettings, asGlobal, ("dde-file-manager", DFMSettings::AppConfig))

DFMApplication *DFMApplicationPrivate::self = nullptr;

DFMApplicationPrivate::DFMApplicationPrivate(DFMApplication *qq)
{
    Q_ASSERT_X(!self, "DFMApplication", "there should be only one application object");
    self = qq;
}

DFMApplication::DFMApplication(QObject *parent)
    : DFMApplication(new DFMApplicationPrivate(this), parent)
{

}

DFMApplication::~DFMApplication()
{

}

DFMApplication *DFMApplication::instance()
{
    return DFMApplicationPrivate::self;
}

DFMSettings *DFMApplication::genericSetting()
{
    if (!gsGlobal.exists() && instance()) {
        gsGlobal->moveToThread(instance()->thread());
    }

    return gsGlobal;
}

DFMSettings *DFMApplication::appSetting()
{
    if (!asGlobal.exists() && instance()) {
        asGlobal->moveToThread(instance()->thread());
    }

    return asGlobal;
}

DFMApplication::DFMApplication(DFMApplicationPrivate *dd, QObject *parent)
    : QObject(parent)
    , d_ptr(dd)
{

}

DFM_END_NAMESPACE
