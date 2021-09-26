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
#ifndef EVENTCALLPROXY_H
#define EVENTCALLPROXY_H

#include "event.h"
#include "eventhandler.h"
#include "dfm-framework/log/frameworklog.h"

#include <QObject>
#include <QMutex>
#include <QDebug>

#include <functional>
#include <memory>


DPF_BEGIN_NAMESPACE
class EventCallProxy final
{
    template <typename T>
    friend class AutoEventHandlerRegister;
    struct HandlerInfo;
    using CreateFunc = std::function<EventHandler*()> ;
    using ExportFunc = std::function<void(HandlerInfo &info, const Event &event)>;

    struct HandlerInfo
    {
        QString className;
        EventHandler* handler {nullptr};
        ExportFunc invoke;
        QStringList topics;
        QFuture<void> future;
    };

public:
    EventCallProxy() = delete;
    static bool pubEvent(const Event& event);
    static bool removeHandler(const QString &className);
    static void removeAllHandlers();

private:
    static void registerHandler(EventHandler::Type type, const QStringList &topics, CreateFunc creator);
    static QList<HandlerInfo> &getInfoList();
    static void fillInfo(HandlerInfo &info, CreateFunc creator);
    static QMutex *eventMutex();
};

// auto register all event handler
template <typename T>
bool AutoEventHandlerRegister<T>::isRegistered = AutoEventHandlerRegister<T>::trigger();
template <typename T>
bool AutoEventHandlerRegister<T>::trigger()
{
    qInfo() << "Register: " << __PRETTY_FUNCTION__;
    EventCallProxy::registerHandler(T::type(), T::topics(), [] { return new T(); });
    return true;
}

DPF_END_NAMESPACE

#endif // EVENTCALLPROXY_H
