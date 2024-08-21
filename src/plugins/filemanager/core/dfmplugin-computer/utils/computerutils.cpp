// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "computerutils.h"
#include "fileentity/entryfileentities.h"
#include "utils/computerdatastruct.h"
#include "deviceproperty/devicepropertydialog.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/dbusservice/global_server_defines.h>
#include <dfm-base/file/entry/entryfileinfo.h>
#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>

#include <dfm-framework/dpf.h>

#include <QtConcurrent>
#include <QApplication>

#include <unistd.h>

Q_DECLARE_METATYPE(QList<QUrl> *)

using namespace dfmplugin_computer;
DFMBASE_USE_NAMESPACE

bool ComputerUtils::contextMenuEnabled = true;

QUrl ComputerUtils::makeBlockDevUrl(const QString &id)
{
    QUrl devUrl;
    devUrl.setScheme(Global::Scheme::kEntry);
    auto shortenBlk = id;
    shortenBlk.remove(QString(DeviceId::kBlockDeviceIdPrefix));   // /org/freedesktop/UDisks2/block_devices/sda1 -> sda1
    auto path = QString("%1.%2").arg(shortenBlk).arg(SuffixInfo::kBlock);   // sda1.blockdev
    devUrl.setPath(path);   // entry:sda1.blockdev
    return devUrl;
}

QString ComputerUtils::getBlockDevIdByUrl(const QUrl &url)
{
    if (url.scheme() != Global::Scheme::kEntry)
        return "";
    if (!url.path().endsWith(SuffixInfo::kBlock))
        return "";

    QString suffix = QString(".%1").arg(SuffixInfo::kBlock);
    QString id = QString("%1%2").arg(DeviceId::kBlockDeviceIdPrefix).arg(url.path().remove(suffix));
    return id;
}

QUrl ComputerUtils::makeProtocolDevUrl(const QString &id)
{
    QUrl devUrl;
    devUrl.setScheme(Global::Scheme::kEntry);
    QString path = QString("%1.%2").arg(QString(id)).arg(SuffixInfo::kProtocol);
    devUrl.setPath(path);
    return devUrl;
}

QString ComputerUtils::getProtocolDevIdByUrl(const QUrl &url)
{
    if (url.scheme() != Global::Scheme::kEntry)
        return "";
    if (!url.path().endsWith(SuffixInfo::kProtocol))
        return "";

    QString suffix = QString(".%1").arg(SuffixInfo::kProtocol);
    return url.path().remove(suffix);
}

QUrl ComputerUtils::makeAppEntryUrl(const QString &filePath)
{
    if (!filePath.startsWith(StandardPaths::location(StandardPaths::kExtensionsAppEntryPath)))
        return {};
    if (!filePath.endsWith(".desktop"))
        return {};

    QString fileName = filePath.mid(filePath.lastIndexOf("/") + 1);
    fileName.remove(".desktop");
    QString newPath = QString("%1.%2").arg(fileName).arg(SuffixInfo::kAppEntry);

    QUrl url;
    url.setScheme(Global::Scheme::kEntry);
    url.setPath(newPath);
    return url;
}

QUrl ComputerUtils::getAppEntryFileUrl(const QUrl &entryUrl)
{
    if (!entryUrl.isValid())
        return {};
    if (!entryUrl.path().endsWith(SuffixInfo::kAppEntry))
        return {};

    QString fileName = entryUrl.path().remove("." + QString(SuffixInfo::kAppEntry));
    QUrl origUrl;
    origUrl.setScheme(Global::Scheme::kFile);
    origUrl.setPath(QString("%1/%2.%3").arg(StandardPaths::location(StandardPaths::kExtensionsAppEntryPath)).arg(fileName).arg("desktop"));
    return origUrl;
}

QUrl ComputerUtils::makeLocalUrl(const QString &path)
{
    QUrl u;
    u.setScheme(Global::Scheme::kFile);
    u.setPath(path);
    return u;
}

QUrl ComputerUtils::makeBurnUrl(const QString &id)
{
    QString dev = id.mid(id.lastIndexOf("/") + 1);
    QUrl u;
    u.setScheme(Global::Scheme::kBurn);
    // burn:///dev/sr0/disc_files/
    u.setPath(QString("/dev/%1/disc_files/").arg(dev));
    return u;
}

quint64 ComputerUtils::getWinId(QWidget *widget)
{
    return FMWindowsIns.findWindowId(widget);
}

bool ComputerUtils::isPresetSuffix(const QString &suffix)
{
    static const QStringList kPresetSuffix { SuffixInfo::kBlock, SuffixInfo::kProtocol, SuffixInfo::kUserDir, SuffixInfo::kAppEntry };
    return kPresetSuffix.contains(suffix);
}

bool ComputerUtils::shouldSystemPartitionHide()
{
    return Application::instance()->genericAttribute(Application::kHiddenSystemPartition).toBool();
}

bool ComputerUtils::shouldLoopPartitionsHide()
{
    return Application::instance()->genericAttribute(Application::kHideLoopPartitions).toBool();
}

bool ComputerUtils::sortItem(const QUrl &a, const QUrl &b)
{
    if (a.scheme() != Global::Scheme::kEntry || b.scheme() != Global::Scheme::kEntry)
        return false;
    DFMEntryFileInfoPointer infoA(new EntryFileInfo(a));
    DFMEntryFileInfoPointer infoB(new EntryFileInfo(b));
    return sortItem(infoA, infoB);
}

bool ComputerUtils::sortItem(DFMEntryFileInfoPointer a, DFMEntryFileInfoPointer b)
{
    if (a && b) {
        if (a->order() == b->order())   // then sort by name
            return a->displayName() < b->displayName();
        else
            return a->order() < b->order();
    }
    return false;
}

int ComputerUtils::getUniqueInteger()
{
    static int idx = 0;
    return ++idx;
}

QMutex ComputerUtils::mtxForCheckGvfs;
QWaitCondition ComputerUtils::condForCheckGvfs;
bool ComputerUtils::checkGvfsMountExist(const QUrl &url, int timeout)
{
    if (!FileUtils::isGvfsFile(url))
        return true;
    setCursorState(true);

    std::string path = url.path().toStdString();
    bool exist = false;
    auto &&fu = QtConcurrent::run([path, &exist] {
        QThread::msleep(100);
        int ret = access(path.c_str(), F_OK);
        exist = (ret == 0);

        fmInfo() << "gvfs path: " << path.c_str() << ", exist: " << exist << ", error: " << strerror(errno);

        exist = true;
        QMutexLocker lk(&mtxForCheckGvfs);
        condForCheckGvfs.wakeAll();
    });

    QMutexLocker lk(&mtxForCheckGvfs);
    condForCheckGvfs.wait(&mtxForCheckGvfs, timeout);
    fu.cancel();
    setCursorState(false);

    if (!exist) {
        auto dirName = url.path().mid(url.path().lastIndexOf("/") + 1);
        DialogManagerInstance->showErrorDialog(QObject::tr("Cannot access"), dirName);
    }

    return exist;
}

void ComputerUtils::setCursorState(bool busy)
{
    if (busy)
        QApplication::setOverrideCursor(Qt::WaitCursor);
    else
        QApplication::restoreOverrideCursor();
}

QStringList ComputerUtils::allValidBlockUUIDs()
{
    const auto &allBlocks = DevProxyMng->getAllBlockIds(GlobalServerDefines::DeviceQueryOption::kNotIgnored).toSet();
    QSet<QString> uuids;
    std::for_each(allBlocks.cbegin(), allBlocks.cend(), [&](const QString &devId) {
        const auto &&data = DevProxyMng->queryBlockInfo(devId);
        const auto &&uuid = data.value(GlobalServerDefines::DeviceProperty::kUUID).toString();
        // optical item not hidden by dconfig, its uuid might be empty.
        if (data.value(GlobalServerDefines::DeviceProperty::kOpticalDrive).toBool())
            return;
        if (!uuid.isEmpty())
            uuids << uuid;
    });
    return uuids.values();
}

QList<QUrl> ComputerUtils::blkDevUrlByUUIDs(const QStringList &uuids)
{
    const auto &&devIds = DevProxyMng->getAllBlockIdsByUUID(uuids);
    QList<QUrl> ret;
    for (const auto &id : devIds)
        ret << makeBlockDevUrl(id);
    return ret;
}

/*!
 * \brief 解析所有插件的配置文件，获取插件配置的计算机预定义数据，用于界面的预显示
 * \return
 */
QList<QVariantMap> ComputerUtils::allPreDefineItemCustomDatas()
{
    DPF_USE_NAMESPACE
    QList<QVariantMap> list;
    LifeCycle::pluginMetaObjs([&list](PluginMetaObjectPointer ptr) {
        Q_ASSERT(ptr);
        const auto &data { ptr->customData() };
        if (data.isEmpty())
            return false;
        const auto &array { ptr->customData().value("ComputerDisplay").toJsonArray() };
        if (array.isEmpty())
            return false;
        for (int i = 0; i != array.count(); ++i) {
            const auto &obj { array.at(i).toObject() };
            list.append(obj.toVariantMap());
        }
        return true;
    });

    return list;
}

QString ComputerUtils::deviceTypeInfo(DFMEntryFileInfoPointer info)
{
    DFMBASE_USE_NAMESPACE
    switch (info->order()) {
    case AbstractEntryFileEntity::kOrderUserDir:
        return QObject::tr("User directory");
    case AbstractEntryFileEntity::kOrderSysDiskRoot:
    case AbstractEntryFileEntity::kOrderSysDiskData:
    case AbstractEntryFileEntity::kOrderSysDisks:
        return QObject::tr("Local disk");
    case AbstractEntryFileEntity::kOrderRemovableDisks:
        return QObject::tr("Removable disk");
    case AbstractEntryFileEntity::kOrderOptical:
        return QObject::tr("DVD");
    case AbstractEntryFileEntity::kOrderSmb:
    case AbstractEntryFileEntity::kOrderFtp:
        return QObject::tr("Network shared directory");
    case AbstractEntryFileEntity::kOrderMTP:
        return QObject::tr("Android mobile device");
    case AbstractEntryFileEntity::kOrderGPhoto2:
        if (getProtocolDevIdByUrl(info->urlOf(UrlInfoType::kUrl)).contains("Apple_Inc"))
            return QObject::tr("Apple mobile device");
        return QObject::tr("Android mobile device");
    case AbstractEntryFileEntity::kOrderFiles:
        //        return QObject::tr("");
    default:
        return QObject::tr("Unknown device");
    }
}

QWidget *ComputerUtils::devicePropertyDialog(const QUrl &url)
{
    QUrl devUrl = convertToDevUrl(url);
    if (devUrl.isEmpty())
        return nullptr;

    DFMEntryFileInfoPointer info(new EntryFileInfo(devUrl));
    DevicePropertyDialog *dialog = new DevicePropertyDialog;
    DeviceInfo devInfo;
    devInfo.icon = info->fileIcon();
    devInfo.deviceUrl = info->urlOf(UrlInfoType::kUrl);
    devInfo.mountPoint = info->targetUrl();
    devInfo.deviceName = info->displayName();
    devInfo.deviceType = ComputerUtils::deviceTypeInfo(info);
    devInfo.fileSystem = info->extraProperty(GlobalServerDefines::DeviceProperty::kFileSystem).toString();
    devInfo.totalCapacity = info->sizeTotal();
    devInfo.availableSpace = info->sizeFree();
    devInfo.deviceDesc = info->extraProperty(GlobalServerDefines::DeviceProperty::kDevice).toString().mid(5);
    dialog->setSelectDeviceInfo(devInfo);
    return dialog;
}

QUrl ComputerUtils::convertToDevUrl(const QUrl &url)
{
    if (url.scheme() == Global::Scheme::kEntry)
        return url;

    QUrl converted = url;
    QList<QUrl> urls {};
    bool ok = UniversalUtils::urlsTransformToLocal({ converted }, &urls);

    if (ok && !urls.isEmpty())
        converted = urls.first();

    QString devId;
    if (converted.scheme() == Global::Scheme::kFile && DevProxyMng->isMptOfDevice(converted.path(), devId)) {
        if (devId.startsWith(kBlockDeviceIdPrefix))
            converted = ComputerUtils::makeBlockDevUrl(devId);
        else
            converted = ComputerUtils::makeProtocolDevUrl(devId);
    } else if (!converted.isValid() && url.scheme() == Global::Scheme::kBurn) {
        // empty disc do not have mapped mount path.
        auto path = url.path();
        QRegularExpression re("^/dev/(.*)/disc_files/");
        auto match = re.match(path);
        if (match.hasMatch() && path.remove(re).isEmpty()) {
            auto vol = match.captured(1);
            auto id = kBlockDeviceIdPrefix + vol;
            converted = ComputerUtils::makeBlockDevUrl(id);
        }
    } else {
        converted = QUrl();   // make it invalid to got handled by default property dialog
    }

    fmDebug() << "convert url from" << url << "to" << converted;
    return converted;
}
