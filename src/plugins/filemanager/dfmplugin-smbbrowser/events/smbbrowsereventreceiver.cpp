// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "smbbrowsereventreceiver.h"
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/utils/systempathutil.h>
#include <dfm-base/dfm_global_defines.h>

#include <dfm-framework/dpf.h>

#include <QDebug>
#include <QRegularExpression>

using namespace dfmplugin_smbbrowser;
DFMBASE_USE_NAMESPACE

SmbBrowserEventReceiver *SmbBrowserEventReceiver::instance()
{
    static SmbBrowserEventReceiver instance;
    return &instance;
}

bool SmbBrowserEventReceiver::detailViewIcon(const QUrl &url, QString *iconName)
{
    if (!iconName)
        return false;

    if (UniversalUtils::urlEquals(url, QUrl(QString("%1:///").arg(Global::Scheme::kNetwork)))) {
        *iconName = SystemPathUtil::instance()->systemPathIconName("Network");
        if (!iconName->isEmpty())
            return true;
    }
    return false;
}

bool SmbBrowserEventReceiver::cancelDelete(quint64, const QList<QUrl> &urls)
{
    if (urls.first().scheme() != DFMBASE_NAMESPACE::Global::Scheme::kSmb
        && urls.first().scheme() != DFMBASE_NAMESPACE::Global::Scheme::kFtp
        && urls.first().scheme() != DFMBASE_NAMESPACE::Global::Scheme::kSFtp) {
        qDebug() << "SmbBrowser could't delete";
        return false;
    }
    return true;
}

bool SmbBrowserEventReceiver::hookSetTabName(const QUrl &url, QString *tabName)
{
    if (!tabName)
        return false;

    if (UniversalUtils::urlEquals(url, QUrl("network:///"))) {
        *tabName = QObject::tr("Computers in LAN");
        return true;
    }

    if (url.scheme() == "smb" && url.path().contains(QRegularExpression(R"([^/]*)"))) {
        auto path = url.toString();
        while (path.endsWith("/"))
            path.chop(1);
        *tabName = path;
        return true;
    }
    return false;
}

SmbBrowserEventReceiver::SmbBrowserEventReceiver(QObject *parent)
    : QObject(parent) { }
