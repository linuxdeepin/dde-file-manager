// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "historystack.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/networkutils.h>

#include <dfm-io/dfile.h>

#include <QDebug>
#include <QProcess>

using namespace dfmplugin_titlebar;
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

        const auto &fileInfo = InfoFactory::create<FileInfo>(url);

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

        const auto &fileInfo = InfoFactory::create<FileInfo>(url);

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

void HistoryStack::removeUrl(const QUrl &url)
{
    if (list.isEmpty() || index < 0 || index >= list.length())
        return;

    const QUrl &curUrl = list.at(index);
    if (UniversalUtils::urlEquals(url, curUrl))
        return;

    QString removePath = url.path();

    if (list.contains(url)) {
        int removeIndex = list.indexOf(url);
        if (index < removeIndex) {
            QList<QUrl> newList = list.mid(0, removeIndex);
            list = newList;
        }

        if (index > removeIndex) {
            QList<QUrl> newList = list.mid(0, removeIndex);
            newList.append(curUrl);
            list = newList;
            index = newList.length() - 1;
        }
    }
}

int HistoryStack::currentIndex()
{
    return index;
}

bool HistoryStack::backIsExist()
{
    if (index <= 0)
        return false;

    const QUrl &backUrl = list.at(index - 1);
    if (!needCheckExist(backUrl))
        return true;

    return checkPathIsExist(backUrl);
}

bool HistoryStack::forwardIsExist()
{
    if (index >= list.size() - 1)
        return false;

    const QUrl &forwardUrl = list.at(index + 1);
    if (!needCheckExist(forwardUrl))
        return true;

    return checkPathIsExist(forwardUrl);
}

bool HistoryStack::needCheckExist(const QUrl &url)
{
    using namespace DFMGLOBAL_NAMESPACE::Scheme;

    const auto &scheme = url.scheme();
    if (scheme == kComputer || scheme == kUserShare)
        return false;

    if (!UrlRoute::hasScheme(scheme))
        return false;

    return true;
}

bool HistoryStack::checkPathIsExist(const QUrl &url)
{
    if (FileUtils::isGvfsFile(url) && NetworkUtils::instance()->checkFtpOrSmbBusy(url)) {
        return  false;
    } else {
        auto info = InfoFactory::create<FileInfo>(url);
        return info && info->exists();
    }
}
