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
#ifndef PLUGINSERVICE_H
#define PLUGINSERVICE_H

#include "qtclassfactory.h"
#include "qtclassmanager.h"
#include "dfm-framework/log/frameworklog.h"
#include "dfm-framework/dfm_framework_global.h"

#include <QObject>
#include <QMetaObject>
#include <QHash>
#include <QDebug>

DPF_BEGIN_NAMESPACE

class PluginService : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(PluginService)
public:
    explicit PluginService(){}
    virtual ~PluginService(){}
}; //接口类

class PluginServiceContext final : public QObject,
        public QtClassFactory<PluginService>,
        public QtClassManager<PluginService>
{
    Q_OBJECT
    inline static PluginServiceContext* ins = nullptr;

public:
    static PluginServiceContext &instance()
    {
        if (!ins)
            ins  = new PluginServiceContext();
        return *ins;
    }

    template<class CT>
    static CT *service(const QString &serviceName)
    {
        return qobject_cast<CT*>(PluginServiceContext::instance().
                                QtClassManager<PluginService>::value(serviceName));
    }

    static QStringList services()
    {
        return PluginServiceContext::instance().keys();
    }

private:
    explicit PluginServiceContext(){}
};

#define PLUGIN_SERVICE(x) \
    static bool x##_regResult = dpf::PluginServiceContext::instance().regClass<x>(#x);

#define IMPORT_SERVICE(x) \
    QString errorString; \
    auto x##_ins = dpf::PluginServiceContext::instance().create(#x,&errorString); \
    if (!x##_ins) \
        dpfCritical() << errorString; \
    else { \
        dpfCritical() << "IMPORT_SERVICE:" << #x;\
        auto appendRes = dpf::PluginServiceContext::instance().append(#x, x##_ins, &errorString);\
        if (!appendRes) dpfCritical()<< errorString;\
    }

#define EXPORT_SERVICE(x) \
    dpf::PluginServiceContext::instance().remove(#x);\
    dpfCritical() << "EXPORT_SERVICE:" << #x;

DPF_END_NAMESPACE

#endif // PLUGINSERVICE_H
