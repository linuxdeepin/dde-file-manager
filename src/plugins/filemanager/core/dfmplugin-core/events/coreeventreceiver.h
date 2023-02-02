/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#ifndef COREEVENTRECEIVER_H
#define COREEVENTRECEIVER_H

#include "dfmplugin_core_global.h"

#include <QObject>

DPCORE_BEGIN_NAMESPACE

class CoreEventReceiver final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(CoreEventReceiver)

public:
    static CoreEventReceiver *instance();

public slots:
    void handleChangeUrl(quint64 windowId, const QUrl &url);
    void handleOpenWindow(const QUrl &url);
    void handleOpenWindow(const QUrl &url, const QVariant &opt);
    void handleLoadPlugins(const QStringList &names);

private:
    explicit CoreEventReceiver(QObject *parent = nullptr);
};

DPCORE_END_NAMESPACE

#endif   // COREEVENTRECEIVER_H
