// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "computereventreceiver.h"
#include "computereventcaller.h"
#include "controller/computercontroller.h"
#include "utils/computerutils.h"
#include "fileentity/protocolentryfileentity.h"

#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/universalutils.h>

#include <QDebug>

using namespace dfmplugin_computer;
DFMBASE_USE_NAMESPACE

ComputerEventReceiver *ComputerEventReceiver::instance()
{
    static ComputerEventReceiver ins;
    return &ins;
}

void ComputerEventReceiver::handleItemEject(const QUrl &url)
{
    ComputerControllerInstance->actEject(url);
}

bool ComputerEventReceiver::handleSepateTitlebarCrumb(const QUrl &url, QList<QVariantMap> *mapGroup)
{
    Q_ASSERT(mapGroup);
    if (url.scheme() == ComputerUtils::scheme()) {
        QVariantMap map;
        map["CrumbData_Key_Url"] = url;
        map["CrumbData_Key_DisplayText"] = tr("Computer");
        map["CrumbData_Key_IconName"] = ComputerUtils::icon().name();
        mapGroup->push_back(map);
        return true;
    } else if (url.scheme() == Global::Scheme::kFile) {
        return parseCifsMountCrumb(url, mapGroup);
    }

    return false;
}

bool ComputerEventReceiver::handleSortItem(const QString &group, const QString &subGroup, const QUrl &a, const QUrl &b)
{
    if (!(group == "Group_Device" || group == "Group_Network"))
        return false;

    if (!(subGroup == Global::Scheme::kComputer || subGroup == Global::Scheme::kSmb || subGroup == Global::Scheme::kFtp))
        return false;

    return ComputerUtils::sortItem(a, b);
}

bool ComputerEventReceiver::handleSetTabName(const QUrl &url, QString *tabName)
{
    // if the url is the mount point of inner disk, set it to alias if alias if not empty.
    auto devs = DevProxyMng->getAllBlockIds(GlobalServerDefines::kMounted | GlobalServerDefines::kSystem);
    for (const auto &dev : devs) {
        auto devInfo = DevProxyMng->queryBlockInfo(dev);
        auto mpt = QUrl::fromLocalFile(devInfo.value(GlobalServerDefines::DeviceProperty::kMountPoint).toString());
        if (UniversalUtils::urlEquals(mpt, url)) {
            auto info = InfoFactory::create<EntryFileInfo>(ComputerUtils::makeBlockDevUrl(dev));
            if (info) {
                *tabName = info->displayName();
                return true;
            }
        }
    }
    return false;
}

void ComputerEventReceiver::setContextMenuEnable(bool enable)
{
    ComputerUtils::contextMenuEnabled = enable;
}

bool ComputerEventReceiver::parseCifsMountCrumb(const QUrl &url, QList<QVariantMap> *mapGroup)
{
    Q_ASSERT(mapGroup);
    const QString &filePath = url.path();
    static const QRegularExpression kCifsPrefix(R"(^/media/.*/smbmounts)");
    auto match = kCifsPrefix.match(filePath);
    if (!match.hasMatch())
        return false;

    QVariantMap rootNode { { "CrumbData_Key_Url", QUrl::fromLocalFile(match.captured()) },
                           { "CrumbData_Key_IconName", "drive-harddisk-symbolic" },
                           { "CrumbData_Key_DisplayText", "" } };
    mapGroup->push_back(rootNode);

    static const QRegularExpression kCifsDevId(R"(^/media/.*/smbmounts/[^/]*)");
    match = kCifsDevId.match(filePath);
    if (!match.hasMatch())
        return true;

    const QString &devPath = match.captured();
    QString host, share;
    bool ok = DeviceUtils::parseSmbInfo(devPath, host, share);
    QString devName = ok ? ProtocolEntryFileEntity::tr("%1 on %2").arg(share).arg(host)
                         : QFileInfo(devPath).fileName();

    QVariantMap devNode { { "CrumbData_Key_Url", QUrl::fromLocalFile(match.captured()) },
                          { "CrumbData_Key_DisplayText", devName } };
    mapGroup->push_back(devNode);

    // parse dir nodes.
    auto path = filePath;
#if (QT_VERSION <= QT_VERSION_CHECK(5, 15, 0))
    QStringList subPaths = path.remove(kCifsDevId).split("/", QString::SkipEmptyParts);
#else
    QStringList subPaths = path.remove(kCifsDevId).split("/", Qt::SkipEmptyParts);
#endif

    QString currPrefix = devPath;
    while (subPaths.count() > 0) {
        QString dirName = subPaths.takeFirst();
        currPrefix = currPrefix + "/" + dirName;
        QVariantMap dirNode { { "CrumbData_Key_Url", QUrl::fromLocalFile(currPrefix) },
                              { "CrumbData_Key_DisplayText", dirName } };
        mapGroup->push_back(dirNode);
    }
    return true;
}

ComputerEventReceiver::ComputerEventReceiver(QObject *parent)
    : QObject(parent)
{
}
