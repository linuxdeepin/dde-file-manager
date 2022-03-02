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
#ifndef FRAMESERVICEPRIVATE_H
#define FRAMESERVICEPRIVATE_H

#include "frame/frameservice.h"

#include <interfaces/abstractdesktopframe.h>

#include <QObject>

DSB_D_BEGIN_NAMESPACE

class FrameServicePrivate : public QObject
{
    Q_OBJECT
public:
    explicit FrameServicePrivate(FrameService *parent);

signals:

public slots:
    bool setProxy(QObject *);
public:
    DFMBASE_NAMESPACE::AbstractDesktopFrame *proxy = nullptr;
private:
    FrameService *q;
};

DSB_D_END_NAMESPACE

#endif // FRAMESERVICEPRIVATE_H
