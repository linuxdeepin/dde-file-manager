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
#include "shareeventhelper.h"
#include "shareeventscaller.h"
#include "utils/shareutils.h"

#include "dfm-base/dfm_global_defines.h"

#include <QDebug>

DPMYSHARES_USE_NAMESPACE

ShareEventHelper *dfmplugin_myshares::ShareEventHelper::instance()
{
    static ShareEventHelper ins;
    return &ins;
}

bool ShareEventHelper::blockPaste(quint64, const QUrl &to)
{
    if (to.scheme() == ShareUtils::scheme()) {
        qDebug() << "paste event is blocked, trying to paste to MyShares";
        return true;
    }
    return false;
}

bool ShareEventHelper::blockDelete(quint64, const QList<QUrl> &urls)
{
    if (containsShareUrl(urls)) {
        qDebug() << "delete event is blocked, trying to delete usershare:///*";
        return true;
    }
    return false;
}

bool ShareEventHelper::blockMoveToTrash(quint64, const QList<QUrl> &urls)
{
    if (containsShareUrl(urls)) {
        qDebug() << "move to trash event is blocked, trying to delete usershare:///*";
        return true;
    }
    return false;
}

bool ShareEventHelper::hookSendOpenWindow(const QList<QUrl> &urls)
{
    bool hook = false;
    for (auto u : urls) {
        if (u.scheme() == ShareUtils::scheme() && u.path() != "/") {
            hook = true;
            break;
        }
    }

    if (hook) {
        ShareEventsCaller::sendOpenDirs(0, urls, ShareEventsCaller::kOpenInNewWindow);
        return true;
    }

    return false;
}

bool ShareEventHelper::hookSendChangeCurrentUrl(quint64 winId, const QUrl &url)
{
    if (url.scheme() == ShareUtils::scheme() && url.path() != "/") {
        auto u(url);
        u.setScheme(DFMBASE_NAMESPACE::Global::kFile);
        QList<QUrl> urls { u };
        ShareEventsCaller::sendOpenDirs(winId, urls, ShareEventsCaller::kOpenInCurrentWindow);
        return true;
    }
    return false;
}

ShareEventHelper::ShareEventHelper(QObject *parent)
    : QObject(parent)
{
}

bool ShareEventHelper::containsShareUrl(const QList<QUrl> &urls)
{
    auto iter = std::find_if(urls.cbegin(), urls.cend(), [](const QUrl &u) { return u.scheme() == ShareUtils::scheme(); });
    return iter != urls.cend();
}
