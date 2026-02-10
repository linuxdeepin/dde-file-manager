// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "shareeventhelper.h"
#include "shareeventscaller.h"
#include "utils/shareutils.h"

#include <dfm-base/dfm_global_defines.h>

#include <QDebug>

using namespace dfmplugin_myshares;

ShareEventHelper *dfmplugin_myshares::ShareEventHelper::instance()
{
    static ShareEventHelper ins;
    return &ins;
}

bool ShareEventHelper::blockPaste(quint64, const QList<QUrl> &fromUrls, const QUrl &to)
{
    Q_UNUSED(fromUrls)

    if (to.scheme() == ShareUtils::scheme()) {
        fmDebug() << "paste event is blocked, trying to paste to MyShares";
        return true;
    }
    return false;
}

bool ShareEventHelper::blockDelete(quint64, const QList<QUrl> &urls, const QUrl &)
{
    if (containsShareUrl(urls)) {
        fmDebug() << "delete event is blocked, trying to delete usershare:///*";
        return true;
    }
    return false;
}

bool ShareEventHelper::blockMoveToTrash(quint64, const QList<QUrl> &urls, const QUrl &)
{
    if (containsShareUrl(urls)) {
        fmDebug() << "move to trash event is blocked, trying to delete usershare:///*";
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
        u.setScheme(DFMBASE_NAMESPACE::Global::Scheme::kFile);
        QList<QUrl> urls { u };

        fmDebug() << "Hooking change current URL operation - redirecting from" << url << "to file scheme:" << u;
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
