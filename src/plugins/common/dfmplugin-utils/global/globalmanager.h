/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
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
#ifndef GLOBALMANAGER_H
#define GLOBALMANAGER_H

#include "dfmplugin_utils_global.h"
#include "global/globaleventreceiver.h"

#include <QObject>

DPUTILS_BEGIN_NAMESPACE

class GlobalManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(GlobalManager)

public:
    static GlobalManager &instance();

    void init();

private:
    explicit GlobalManager(QObject *parent = nullptr);

private:
    QScopedPointer<GlobalEventReceiver> eventReceiver { new GlobalEventReceiver };
};

DPUTILS_END_NAMESPACE

#endif   // GLOBALMANAGER_H
