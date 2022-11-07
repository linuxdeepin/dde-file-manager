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
#ifndef POLICYKITHELPER_H
#define POLICYKITHELPER_H

#include "daemonplugin_accesscontrol_global.h"

#include <QObject>
#include <polkit-qt5-1/PolkitQt1/Authority>

DAEMONPAC_BEGIN_NAMESPACE

class PolicyKitHelper
{
public:
    inline static PolicyKitHelper *instance()
    {
        static PolicyKitHelper instance;
        return &instance;
    }

    bool checkAuthorization(const QString &actionId, qint64 applicationPid);

private:
    PolicyKitHelper();
    ~PolicyKitHelper();

    Q_DISABLE_COPY(PolicyKitHelper)
};

DAEMONPAC_END_NAMESPACE

#endif   // POLICYKITHELPER_H
