/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *             liuyangming<liuyangming@uniontech.com>
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
#ifndef TRASHSERVICE_H
#define TRASHSERVICE_H

#include "trash_defines.h"
#include <dfm-framework/service/pluginservicecontext.h>

namespace dfm_service_common {

class TrashService final : public dpf::PluginService, dpf::AutoServiceRegister<TrashService>
{
    Q_OBJECT
    Q_DISABLE_COPY(TrashService)
    friend class dpf::QtClassFactory<dpf::PluginService>;

public:
    static QString name()
    {
        return "org.deepin.service.TrashService";
    }
    static TrashService *instance();

    bool isEmpty();

private:
    explicit TrashService(QObject *parent = nullptr);
    virtual ~TrashService() = default;
};

}

#define trashServIns ::dfm_service_common::TrashService::instance()

#endif   // TRASHSERVICE_H
