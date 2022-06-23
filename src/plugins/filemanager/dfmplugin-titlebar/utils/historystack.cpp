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
#include "historystack.h"

#include "dfm-base/base/urlroute.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/dfm_global_defines.h"

#include <QDebug>
#include <QProcess>

DPTITLEBAR_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

HistoryStack::HistoryStack(int threshold)
{
    curThreshold = threshold;
    index = -1;
}

void HistoryStack::append(const QUrl &url)
{
    if ((index < list.count()) && (index >= 0)) {
        if (list.at(index) == url)
            return;
    }

    if (index < curThreshold) {
        ++index;

        if (index != list.size()) {
            list = list.mid(0, index);
        }

        list.append(url);
    } else {
        list.takeFirst();
        list.append(url);
    }
}

QUrl HistoryStack::back()
{
    const QUrl &currentUrl = list.value(index);
    QUrl url;

    if (index <= 0)
        return url;

    while (--index >= 0) {
        if (index >= list.count())
            continue;

        url = list.at(index);

        // TODO(zhangs): check network, computer, cellphone...

        const auto &fileInfo = InfoFactory::create<AbstractFileInfo>(url);

        if (url.scheme() != Global::Scheme::kFile && !fileInfo)
            break;

        if (!fileInfo || !fileInfo->exists() || currentUrl == url) {
            removeAt(index);
            url = list.at(index);
        } else {
            break;
        }
    }

    return url;
}

QUrl HistoryStack::forward()
{
    const QUrl &currentUrl = list.value(index);
    QUrl url;

    if (index >= list.count() - 1)
        return url;

    while (++index < list.count()) {
        url = list.at(index);

        // TODO(zhangs): check network, computer, cellphone...

        const auto &fileInfo = InfoFactory::create<AbstractFileInfo>(url);

        if (url.scheme() != Global::Scheme::kFile && !fileInfo)
            break;

        if (!fileInfo || !fileInfo->exists() || currentUrl == url) {
            removeAt(index);
            --index;
            url = list.at(index);
        } else {
            break;
        }
    }

    return url;
}

void HistoryStack::setThreshold(int threshold)
{
    curThreshold = threshold;
}

bool HistoryStack::isFirst()
{
    if (index < 0) {
        index = 0;
    }
    return index == 0;
}

bool HistoryStack::isLast()
{
    if (index > list.size() - 1) {
        index = list.size() - 1;
    }
    return index == list.size() - 1;
}

int HistoryStack::size()
{
    return list.size();
}

void HistoryStack::removeAt(int i)
{
    list.removeAt(i);
}

int HistoryStack::currentIndex()
{
    return index;
}
