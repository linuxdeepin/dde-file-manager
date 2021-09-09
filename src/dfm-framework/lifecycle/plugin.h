/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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
#ifndef PLUGIN_H
#define PLUGIN_H

#include "dfm-framework/lifecycle/plugincontext.h"
#include "dfm-framework/dfm_framework_global.h"

#include <QObject>
#include <QSharedData>

DPF_BEGIN_NAMESPACE


class PluginContext;

/**
 * @brief The Plugin class
 * 插件接口类，用于实现插件,使用方式如下
 * @code
 *  class Core : public Plugin
 *  {
 *      Q_OBJECT
 *      Q_PLUGIN_METADATA(IID PLUGIN_INTERFACE FILE "core.json")
 *  public:
 *      virtual void initialized() override;
 *      virtual bool start() override;
 *      virtual PluginMetaObject::ShutDownFlag stop() override;
 *  };
 * @endcode
 * Q_PLUGIN_METADATA 可参阅Qt宏定义
 * PLUGIN_INTERFACE
 */
class Plugin : public QObject
{
    Q_OBJECT
public:
    enum ShutdownFlag {
        Synch, /// 同步释放标识
        Asynch, /// 异步释放标识
    };

    explicit Plugin();
    virtual ~Plugin() override;
    virtual void initialize();

    /**
     * @brief start
     * @return bool 如果返回true则表示当前插件start函数执行正常
     * false则代表当前内部执行存在问题
     */
    virtual bool start(QSharedPointer<PluginContext> context) = 0;
    virtual ShutdownFlag stop();

signals:

    /**
     * @brief asyncStopFinished 异步释放完成的信号
     */
    void asyncStopFinished();
};

DPF_END_NAMESPACE

#endif // PLUGIN_H
