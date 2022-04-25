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
#ifndef CANVASMODELEXTEND_P_H
#define CANVASMODELEXTEND_P_H

#include "ddplugin_canvas_global.h"
#include "canvasmodelextend.h"
#include "canvaseventprovider.h"

#include <QObject>

DDP_CANVAS_BEGIN_NAMESPACE

static constexpr char kFilterCanvasModelData[] = "CanvasModel_Filter_data";

class CanvasModelExtendPrivate : public QObject, public CanvasEventProvider
{
    Q_OBJECT
public:
    explicit CanvasModelExtendPrivate(CanvasModelExtend *);
    ~CanvasModelExtendPrivate() override;
protected:
    void registerEvent() override;
private:
    CanvasModelExtend *q;
};

DDP_CANVAS_END_NAMESPACE

#endif // CANVASMODELEXTEND_P_H
