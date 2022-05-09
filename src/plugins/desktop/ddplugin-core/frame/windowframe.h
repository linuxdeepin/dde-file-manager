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
#ifndef WINDOWFRAME_H
#define WINDOWFRAME_H

#include "ddplugin_core_global.h"

#include <interfaces/abstractdesktopframe.h>

DDPCORE_BEGIN_NAMESPACE
class WindowFramePrivate;
class WindowFrame : public DFMBASE_NAMESPACE::AbstractDesktopFrame
{
    Q_OBJECT
    friend class WindowFramePrivate;
public:
    explicit WindowFrame(QObject *parent = nullptr);
    bool init();
    QList<QWidget *> rootWindows() const override;
    void layoutChildren() override;
public slots:
    void buildBaseWindow();
    void onGeometryChanged();
    void onAvailableGeometryChanged();
private:
    WindowFramePrivate *d;
};

DDPCORE_END_NAMESPACE

#endif // WINDOWFRAME_H
