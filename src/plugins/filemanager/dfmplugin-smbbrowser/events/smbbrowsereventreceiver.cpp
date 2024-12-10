// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "smbbrowsereventreceiver.h"
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/utils/systempathutil.h>
#include <dfm-base/base/device/deviceutils.h>
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

bool SmbBrowserEventReceiver::cancelDelete(quint64, const QList<QUrl> &urls, const QUrl &rootUrl)
{
    if (urls.first().scheme() != DFMBASE_NAMESPACE::Global::Scheme::kSmb
        && urls.first().scheme() != DFMBASE_NAMESPACE::Global::Scheme::kFtp
        && urls.first().scheme() != DFMBASE_NAMESPACE::Global::Scheme::kSFtp) {
        fmDebug() << "SmbBrowser could't delete";
        return false;
    }
    // Network Neighborhood dot not use
    if (UniversalUtils::isNetworkRoot(rootUrl)) {
        fmDebug() << "Network Neighborhood view SmbBrowser could't delete";
        return true;
    }
    return true;
}

bool SmbBrowserEventReceiver::cancelMoveToTrash(quint64, const QList<QUrl> &, const QUrl &rootUrl)
{
    // Network Neighborhood dot not use
    if (UniversalUtils::isNetworkRoot(rootUrl)) {
        fmDebug() << "Network Neighborhood view SmbBrowser could't using";
        return true;
    }
    return false;
}

bool SmbBrowserEventReceiver::hookSetTabName(const QUrl &url, QString *tabName)
{
    if (!tabName)
        return false;

    if (UniversalUtils::urlEquals(url, QUrl("network:///"))) {
        *tabName = QObject::tr("Computers in LAN");
        return true;
    }

    static QRegularExpression regx(R"([^/]*)");
    if (url.scheme() == "smb" && url.path().contains(regx)) {
        auto path = url.toString();
        while (path.endsWith("/"))
            path.chop(1);
        *tabName = path;
        return true;
    }
    return false;
}

bool SmbBrowserEventReceiver::hookTitleBarAddrHandle(QUrl *url)
{
    Q_ASSERT(url);
    QUrl in(*url), out;
    if (getOriginalUri(in, &out)) {
        *url = out;
        return true;
    }
    return false;
}

bool SmbBrowserEventReceiver::hookAllowRepeatUrl(const QUrl &cur, const QUrl &pre)
{
    QStringList allowReEnterScehmes { Global::Scheme::kSmb,
                                      Global::Scheme::kSFtp,
                                      Global::Scheme::kFtp,
                                      Global::Scheme::kDav,
                                      Global::Scheme::kDavs,
                                      Global::Scheme::kNfs };
    return allowReEnterScehmes.contains(cur.scheme()) && allowReEnterScehmes.contains(pre.scheme());
}

bool SmbBrowserEventReceiver::getOriginalUri(const QUrl &in, QUrl *out)
{
    QString path = in.path();

    // is cifs
    static const QRegularExpression kCifsPrefix { R"(^/(?:run/)?media/[^/]*/smbmounts/smb-share:[^/]*)" };
    if (path.contains(kCifsPrefix)) {
        QString host, share, port;
        if (!DeviceUtils::parseSmbInfo(path, host, share, &port))
            return false;

        if (out) {
            out->setScheme("smb");
            out->setHost(host);
            if (!port.isEmpty())
                out->setPort(port.toInt());
            QString subPath = "/" + share;
            subPath += path.remove(kCifsPrefix);
            out->setPath(subPath);
            return true;
        }
    }

    // is gvfs: since mtp/gphoto... scheme are not supported path lookup, only handle ftp/sftp/smb
    // use GIO to obtain the original URI
    if (path.contains(QRegularExpression(R"(((^/run/user/[0-9]*/gvfs)|(^/root/.gvfs))/(ftp|sftp|smb|dav|davs|nfs))"))) {
        SyncFileInfo f(in);
        QUrl u = f.urlOf(dfmbase::FileInfo::FileUrlInfoType::kOriginalUrl);
        if (u.isValid() && out) {
            *out = u;
            return true;
        }
    }

    return false;
}

SmbBrowserEventReceiver::SmbBrowserEventReceiver(QObject *parent)
    : QObject(parent) {}
