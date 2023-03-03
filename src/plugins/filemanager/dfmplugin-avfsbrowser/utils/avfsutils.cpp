// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "avfsutils.h"

#include "dfm-base/dfm_global_defines.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/base/device/deviceutils.h"
#include "dfm-base/base/device/deviceproxymanager.h"
#include "dfm-base/base/application/application.h"
#include "dfm-base/utils/universalutils.h"
#include "dfm-base/dbusservice/global_server_defines.h"

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
    auto info = InfoFactory::create<AbstractFileInfo>(url);
    if (!info || info->nameOf(NameInfoType::kMimeTypeName) == kTypeCdImage)
        return false;

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
        qWarning() << "non-local file doesn't support to convert to avfs url";
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
    QString longestLocalParent;
    QList<QVariantMap> ret;
#if (QT_VERSION <= QT_VERSION_CHECK(5, 15, 0))
    QStringList frags { url.path().split("/", QString::SkipEmptyParts) };
#else
    QStringList frags { url.path().split("/", Qt::SkipEmptyParts) };
#endif
    while (!frags.isEmpty()) {
        auto path = frags.join("/").prepend("/");
        auto icon = parseDirIcon(path);
        if (!icon.isEmpty()) {
            longestLocalParent = longestLocalParent.length() < path ? path : longestLocalParent;
            ret.append({ { "CrumbData_Key_Url", QUrl::fromLocalFile(path) },
                         { "CrumbData_Key_IconName", icon } });
            break;
        } else {
            if (longestLocalParent.isEmpty()) {
                auto tmpFrags { frags };
                // if every dir in tmpFrags exists, means no archive in parents,
                // a path ends with '#' may not exist cause it's supported in avfs.
                while (!tmpFrags.isEmpty()) {
                    auto path = tmpFrags.join("/").prepend("/");
                    if (access(path.toStdString().c_str(), F_OK) == 0) {
                        longestLocalParent = path;
                        break;
                    }
                    tmpFrags.removeLast();
                    if (tmpFrags.count() == 0 && longestLocalParent.length() == 0)
                        longestLocalParent = "/";
                }
            }
            auto url = longestLocalParent.length() < path.length() ? makeAvfsUrl(path) : QUrl::fromLocalFile(path);
            ret.append({ { "CrumbData_Key_Url", url },
                         { "CrumbData_Key_DisplayText", frags.last() } });
        }
        frags.removeLast();

        if (frags.isEmpty())
            ret.append({ { "CrumbData_Key_Url", QUrl::fromLocalFile("/") },
                         { "CrumbData_Key_IconName", "drive-harddisk-root-symbolic" } });
    }
    std::reverse(ret.begin(), ret.end());
    return ret;
}

QString AvfsUtils::parseDirIcon(const QString &path)
{
    if (path == QStandardPaths::writableLocation(QStandardPaths::HomeLocation))
        return "user-home";

    auto dev = DeviceUtils::getMountInfo(path, false);

    if (dev.startsWith("/dev/")) {
        if (dev.startsWith("/dev/sr"))
            return "media-optical-symbolic";

        auto id = kBlockDeviceIdPrefix + dev.mid(5);
        auto info = DevProxyMng->queryBlockInfo(id);
        if (info.value(GlobalServerDefines::DeviceProperty::kRemovable).toBool())
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
