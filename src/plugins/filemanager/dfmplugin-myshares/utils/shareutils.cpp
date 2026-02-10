// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "shareutils.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/dfm_global_defines.h>

#include <QDebug>

using namespace dfmplugin_myshares;

ShareUtils *ShareUtils::instance()
{
    static ShareUtils instance;
    return &instance;
}

QString ShareUtils::scheme()
{
    return "usershare";
}

QIcon ShareUtils::icon()
{
    return QIcon::fromTheme("folder-publicshare");
}

QString ShareUtils::displayName()
{
    return QObject::tr("My Shares");
}

QUrl ShareUtils::rootUrl()
{
    return makeShareUrl("/");
}

QUrl ShareUtils::makeShareUrl(const QString &path)
{
    QUrl u;
    u.setScheme(scheme());
    u.setPath(path);
    return u;
}

QUrl ShareUtils::convertToLocalUrl(const QUrl &shareUrl)
{
    if (shareUrl.scheme() != scheme())
        return {};
    QUrl u = shareUrl;
    u.setScheme(DFMBASE_NAMESPACE::Global::Scheme::kFile);
    return u;
}

ShareUtils::ShareUtils(QObject *parent)
    : QObject(parent)
{
}
