/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *             liuyangming<liuyangming@uniontech.com>
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
#include "trashcoreunicastreceiver.h"
#include "utils/trashcorehelper.h"

#include <dfm-framework/framework.h>

DPTRASHCORE_USE_NAMESPACE

#define STR1(s) #s
#define STR2(s) STR1(s)

inline QString topic(const QString &func)
{
    return QString(STR2(DSC_FM_NAMESPACE)) + "::" + func;
}

TrashCoreUnicastReceiver *TrashCoreUnicastReceiver::instance()
{
    static TrashCoreUnicastReceiver receiver;
    return &receiver;
}

void TrashCoreUnicastReceiver::connectService()
{
    dpfInstance.eventUnicast().connect(topic("TrashService::isEmpty"), this, &TrashCoreUnicastReceiver::invokeIsEmpty);
}

bool TrashCoreUnicastReceiver::invokeIsEmpty()
{
    return TrashCoreHelper::isEmpty();
}

TrashCoreUnicastReceiver::TrashCoreUnicastReceiver(QObject *parent)
    : QObject(parent)
{
}
