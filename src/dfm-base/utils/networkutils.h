/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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
#ifndef NETWORKUTILS_H
#define NETWORKUTILS_H

#include "dfm-base/dfm_base_global.h"

#include <QObject>
#include <QString>

#include <functional>

DFMBASE_BEGIN_NAMESPACE

class NetworkUtils : public QObject
{
    Q_OBJECT

public:
    inline static NetworkUtils *instance()
    {
        static NetworkUtils s;
        return &s;
    }

    bool checkNetConnection(const QString &host, const QString &port);
    void doAfterCheckNet(const QString &host, const QString &port, std::function<void(bool)> callback = nullptr);
    bool parseIp(const QString &mpt, QString &ip, QString &port);

protected:
    NetworkUtils(QObject *parent = nullptr);
};

DFMBASE_END_NAMESPACE

#endif   // NETWORKUTILS_H
