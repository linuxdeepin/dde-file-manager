/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
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
#ifndef BACKGROUNDSERVICE_H
#define BACKGROUNDSERVICE_H

#include "services/desktop/dd_service_global.h"
#include "services/desktop/event/eventinformant.h"

#include <dfm-framework/framework.h>

class Core;
DSB_D_BEGIN_NAMESPACE
class BackgroundServicePrivate;
class BackgroundService final : public dpf::PluginService, dpf::AutoServiceRegister<BackgroundService>, public EventInformant
{
    Q_OBJECT
    Q_DISABLE_COPY(BackgroundService)
    friend class dpf::QtClassFactory<dpf::PluginService>;
    friend class ::Core;
public:
    static QString name()
    {
        return "org.deepin.service.BackgroundService";
    }
protected:
    explicit BackgroundService(QObject *parent = nullptr);
    ~BackgroundService();
private:
    BackgroundServicePrivate *d;
};
DSB_D_END_NAMESPACE

#endif // BACKGROUNDSERVICE_H
