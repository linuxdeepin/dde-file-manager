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
#ifndef CANVASBROKER_P_H
#define CANVASBROKER_P_H

#include "canvasmanagerbroker.h"
#include "canvaseventprovider.h"
#include "canvasmanager.h"
#include "model/fileinfomodel.h"

DDP_CANVAS_BEGIN_NAMESPACE

inline constexpr char kSlotCanvasManagerFileModel[] = "CanvasManager_Method_fileInfoModel";
inline constexpr char kSlotCanvasManagerUpdate[] = "CanvasManager_Method_update";
inline constexpr char kSlotCanvasManagerEdit[] = "CanvasManager_Method_edit";

class CanvasManagerBrokerPrivate : public QObject, public CanvasEventProvider
{
    Q_OBJECT
public:
    explicit CanvasManagerBrokerPrivate(CanvasManagerBroker *);

protected:
    void registerEvent() override;

public:
    CanvasManager *canvas = nullptr;

private:
    CanvasManagerBroker *q;
};

DDP_CANVAS_END_NAMESPACE

#endif   // CANVASBROKER_P_H
