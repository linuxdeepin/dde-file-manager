/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#ifndef EVENTHELPERFUNC_H
#define EVENTHELPERFUNC_H

#include "services/desktop/dd_service_global.h"

#include <QVariantHash>

DSB_D_BEGIN_NAMESPACE

class EventHelperFunc
{
public:
static inline int getEventID(const QVariantHash &e, const QString &topic) {
    int ret = -1;
    if (e.contains(topic)) {
        bool ok = false;
        int tmp = e.value(topic).toInt(&ok);
        if (ok)
            ret = tmp;
    }

    return ret;
}
};

DSB_D_END_NAMESPACE


#endif // EVENTHELPERFUNC_H
