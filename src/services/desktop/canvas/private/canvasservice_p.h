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
#ifndef CANVASSERVICE_P_H
#define CANVASSERVICE_P_H

#include "canvas/canvasservice.h"

DSB_D_BEGIN_NAMESPACE

class CanvasServicePrivate : public QObject
{
    Q_OBJECT
public:
    explicit CanvasServicePrivate(CanvasService *parent);
private:
    CanvasService *q;
};

DSB_D_END_NAMESPACE

#endif // CANVASSERVICE_P_H
