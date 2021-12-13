/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *             zhangyu<zhangyub@uniontech.com>
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
#include "canvasservice.h"
#include "filetreater.h"
#include "canvasmanager.h"

DSB_D_BEGIN_NAMESPACE

CanvasService::CanvasService(QObject *parent)
    : PluginService(parent), AutoServiceRegister<CanvasService>()
{
    FileTreaterCt->init();
    canvasMgrProxy = new CanvasManager(this);
    canvasMgrProxy->init();

    // todo background signal
}

CanvasService::~CanvasService()
{
    delete canvasMgrProxy;
    canvasMgrProxy = nullptr;
}

DSB_D_END_NAMESPACE
