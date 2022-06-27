/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#ifndef EMBLEMSERVICE_H
#define EMBLEMSERVICE_H

#include "emblem_defines.h"

#include <QObject>

#include <dfm-framework/framework.h>

namespace dfm_service_common {

class EmblemService final : public dpf::PluginService, dpf::AutoServiceRegister<EmblemService>
{
    Q_OBJECT
    Q_DISABLE_COPY(EmblemService)
    friend class dpf::QtClassFactory<dpf::PluginService>;

public:
    static QString name()
    {
        return "org.deepin.service.EmblemService";
    }

private:
    explicit EmblemService(QObject *parent = nullptr);
};

}

#endif   // EMBLEMSERVICE_H
