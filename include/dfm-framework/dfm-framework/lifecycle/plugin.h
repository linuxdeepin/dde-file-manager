/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
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
#ifndef PLUGIN_H
#define PLUGIN_H

#include <dfm-framework/dfm_framework_global.h>

#include <QObject>
#include <QSharedData>

DPF_BEGIN_NAMESPACE

class PluginContext;
class Plugin : public QObject
{
    Q_OBJECT
public:
    virtual void initialize();

    /*!
     * \brief start
     * \return bool If it returns true, it means that the current plugin start function is executing properly,
     * false that a problem with current internal implementation
     */
    virtual bool start() = 0;

    virtual void stop();

signals:
    void asyncStopFinished();
};

DPF_END_NAMESPACE

#endif   // PLUGIN_H
