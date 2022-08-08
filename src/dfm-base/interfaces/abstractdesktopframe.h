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
#ifndef ABSTRACTDESKTOPFRAME_H
#define ABSTRACTDESKTOPFRAME_H

#include "dfm-base/dfm_base_global.h"
#include "dfm-base/dfm_desktop_defines.h"

#include <QObject>

namespace dfmbase {

class AbstractDesktopFrame : public QObject
{
    Q_OBJECT
public:
    explicit AbstractDesktopFrame(QObject *parent = nullptr);
    virtual QList<QWidget *> rootWindows() const = 0;
    virtual void layoutChildren() = 0;
Q_SIGNALS:   // must connect with Qt::DirectConnection
    void windowAboutToBeBuilded();
    void windowBuilded();
    void windowShowed();
    void geometryChanged();
    void availableGeometryChanged();
public Q_SLOTS:
};

}

#endif   // ABSTRACTDESKTOPFRAME_H
