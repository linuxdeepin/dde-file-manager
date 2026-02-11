// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "avfsutils.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/dbusservice/global_server_defines.h>

#include <QStandardPaths>
#include <QProcess>
#include <QDebug>
#include <QRegularExpression>

#include <unistd.h>

using namespace dfmplugin_avfsbrowser;
DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
using namespace Mime;

AvfsUtils *AvfsUtils::instance()
{
    static AvfsUtils instance;
    return &instance;
}

bool AvfsUtils::isSupportedArchives(const QUrl &url)
{
    auto info = InfoFactory::create<FileInfo>(url);
    if (!info) {
        fmDebug() << "Failed to create FileInfo for URL:" << url;
        return false;
    }

    return supportedArchives().contains(info->nameOf(NameInfoType::kMimeTypeName));
}

bool AvfsUtils::isSupportedArchives(const QString &path)
{
    return isSupportedArchives(QUrl::fromLocalFile(path));
}

bool AvfsUtils::isAvfsMounted()
{
    return !DeviceUtils::getMountInfo("avfsd").isEmpty();
}

void AvfsUtils::mountAvfs()
{
#if (QT_VERSION <= QT_VERSION_CHECK(5, 15, 0))
    QProcess::startDetached("/usr/bin/mountavfs");
#else
    QProcess::startDetached("/usr/bin/mountavfs", {});
#endif
}

void AvfsUtils::unmountAvfs()
{
#if (QT_VERSION <= QT_VERSION_CHECK(5, 15, 0))
    QProcess::startDetached("/usr/bin/umountavfs");
#else
    QProcess::startDetached("/usr/bin/umountavfs", {});
#endif
}

QString AvfsUtils::avfsMountPoint()
{
    return DeviceUtils::getMountInfo("avfsd");
}

bool AvfsUtils::archivePreviewEnabled()
{
    return Application::genericAttribute(Application::GenericAttribute::kPreviewCompressFile).toBool();
}

QUrl AvfsUtils::avfsUrlToLocal(const QUrl &avfsUrl)
{
    if (avfsUrl.scheme() != scheme())
        return avfsUrl;

    auto path = avfsUrl.path();
    path.prepend(avfsMountPoint());
    return QUrl::fromLocalFile(path);
}

QUrl AvfsUtils::localUrlToAvfsUrl(const QUrl &url)
{
    if (!url.path().startsWith(avfsMountPoint()))
        return url;
    auto path = url.path().remove(QRegularExpression("^" + avfsMountPoint()));
    return makeAvfsUrl(path);
}

QUrl AvfsUtils::localArchiveToAvfsUrl(const QUrl &url)
{
    if (url.scheme() != Global::Scheme::kFile) {
        fmWarning() << "non-local file doesn't support to convert to avfs url";
        return url;
    }

    auto path = url.path();
    if (path.startsWith(avfsMountPoint()))
        path.remove(QRegularExpression("^" + avfsMountPoint()));
    while (path.endsWith("/") && path != "/")
        path.chop(1);
    path.append("#");

    QUrl u;
    u.setScheme(scheme());
    u.setPath(path);
    return u;
}

QUrl AvfsUtils::makeAvfsUrl(const QString &path)
{
    QUrl u;
    u.setScheme(scheme());
    u.setPath(path);
    return u;
}

QList<QVariantMap> AvfsUtils::seperateUrl(const QUrl &url)
{
    fmDebug() << "### split current url:" << url;
    QString localPath = url.path();
    if (url.scheme() == scheme())
        localPath = avfsUrlToLocal(url).path();

#if (QT_VERSION <= QT_VERSION_CHECK(5, 15, 0))
    QStringList frags { localPath.split("/", QString::SkipEmptyParts) };
#else
    QStringList frags { localPath.split("/", Qt::SkipEmptyParts) };
#endif

    QList<QVariantMap> ret;
    while (!frags.isEmpty()) {
        auto path = frags.join("/").prepend("/").append("/");
        if (path.contains("#")) {
            ret.append({ { "CrumbData_Key_Url", localUrlToAvfsUrl(QUrl::fromLocalFile(path)) },
                         { "CrumbData_Key_IconName", "" },
                         { "CrumbData_Key_DisplayText", frags.last() } });

            fmDebug() << "# " << localUrlToAvfsUrl(QUrl::fromLocalFile(path));
        } else {
            path.replace(avfsMountPoint() + "/", "/");
            QString icon = parseDirIcon(path);
            ret.append({ { "CrumbData_Key_Url", QUrl::fromLocalFile(path) },
                         { "CrumbData_Key_IconName", icon },
                         { "CrumbData_Key_DisplayText", qApp->translate("QObject", frags.last().toStdString().c_str()) } });

            fmDebug() << "# " << QUrl::fromLocalFile(path);

            if (!icon.isEmpty())   // root node is found.
                break;
        }
        frags.removeLast();
    }
    std::reverse(ret.begin(), ret.end());

    return ret;
}

QString AvfsUtils::parseDirIcon(QString path)
{
    while (path.endsWith("/") && path != "/")
        path.chop(1);

    if (path == QStandardPaths::writableLocation(QStandardPaths::HomeLocation))
        return "user-home";

    auto dev = DeviceUtils::getMountInfo(path, false);

    if (dev.startsWith("/dev/")) {
        if (dev.startsWith("/dev/sr"))
            return "media-optical-symbolic";

        auto id = kBlockDeviceIdPrefix + dev.mid(5);
        auto info = DevProxyMng->queryBlockInfo(id);
        if (info.value(GlobalServerDefines::DeviceProperty::kEjectable).toBool()
            && info.value(GlobalServerDefines::DeviceProperty::kCanPowerOff).toBool())
            return "drive-removable-media-symbolic";
        else
            return "drive-harddisk-symbolic";
    } else if (dev == "gvfsd-fuse") {
        return "drive-harddisk-symbolic";
    } else {
        return "";
    }
}

AvfsUtils::AvfsUtils(QObject *parent)
    : QObject(parent)
{
}
