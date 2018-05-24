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
#ifndef DFMAPPLICATION_H
#define DFMAPPLICATION_H

#include <QObject>

#include <dfmglobal.h>

DFM_BEGIN_NAMESPACE

class DFMSettings;
class DFMApplicationPrivate;
class DFMApplication : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(DFMApplication)

public:
    explicit DFMApplication(QObject *parent = nullptr);
    ~DFMApplication();

    static DFMApplication *instance();

    static DFMSettings *genericSetting();
    static DFMSettings *appSetting();

protected:
    DFMApplication(DFMApplicationPrivate *dd, QObject *parent = nullptr);

private:
    QScopedPointer<DFMApplicationPrivate> d_ptr;
};

DFM_END_NAMESPACE

#endif // DFMAPPLICATION_H
