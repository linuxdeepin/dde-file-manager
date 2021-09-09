/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyu@uniontech.com>
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
#ifndef PLUGINCONTEXT_H
#define PLUGINCONTEXT_H

#include "dfm-framework/service/pluginserviceglobal.h"
#include "dfm-framework/dfm_framework_global.h"

#include <QMetaType>
#include <QMetaObject>
#include <QMetaClassInfo>
#include <QObject>
#include <QQueue>

DPF_BEGIN_NAMESPACE

class PluginContext final : public QSharedData
{

public:
    explicit PluginContext(){}

    virtual ~PluginContext(){}

    template<class CT>
    CT *service(const QString &serviceName)
    {
        auto val = GlobalPrivate::PluginServiceGlobal::instance().value(serviceName);
        return qobject_cast<CT*>(val);
    }

    QStringList services()
    {
        return GlobalPrivate::PluginServiceGlobal::instance().keys();
    }
};

DPF_END_NAMESPACE

#endif // PLUGINCONTEXT_H
