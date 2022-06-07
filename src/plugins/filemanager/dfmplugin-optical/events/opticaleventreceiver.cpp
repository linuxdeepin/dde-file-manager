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
#include "opticaleventreceiver.h"
#include "utils/opticalhelper.h"

DPOPTICAL_USE_NAMESPACE

OpticalEventReceiver &OpticalEventReceiver::instance()
{
    static OpticalEventReceiver ins;
    return ins;
}

bool OpticalEventReceiver::handleDeleteFilesShortcut(quint64, const QList<QUrl> &urls)
{
    auto iter = std::find_if(urls.cbegin(), urls.cend(), [](const QUrl &url) {
        return OpticalHelper::burnIsOnDisc(url);
    });
    if (iter != urls.cend()) {
        qInfo() << "delete event is blocked, trying to delete disc burn:///*";
        return true;
    }
    return false;
}

OpticalEventReceiver::OpticalEventReceiver(QObject *parent)
    : QObject(parent)
{
}
