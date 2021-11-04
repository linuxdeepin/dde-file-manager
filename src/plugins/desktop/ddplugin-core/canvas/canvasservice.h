/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
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
#ifndef CANVASSERVICE_H
#define CANVASSERVICE_H

#include "dfm_desktop_service_global.h"

#include <dfm-framework/framework.h>

DSB_D_BEGIN_NAMESPACE
class CanvasService final : public dpf::PluginService, dpf::AutoServiceRegister<CanvasService>
{
    Q_OBJECT
    Q_DISABLE_COPY(CanvasService)
    friend class dpf::QtClassFactory<dpf::PluginService>;
public:
    static QString name()
    {
        return "org.deepin.service.CanvasService";
    }
};
DSB_D_END_NAMESPACE

#endif // CANVASSERVICE_H
