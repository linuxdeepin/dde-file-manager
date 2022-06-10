/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
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
#include "computerutils.h"
#include "fileentity/entryfileentities.h"
#include "utils/computerdatastruct.h"
#include "deviceproperty/devicepropertydialog.h"

#include "services/common/delegate/delegateservice.h"
#include "services/filemanager/windows/windowsservice.h"

#include "dfm-base/dfm_global_defines.h"
#include "dfm-base/dbusservice/global_server_defines.h"
#include "dfm-base/file/entry/entryfileinfo.h"
#include "dfm-base/base/urlroute.h"
#include "dfm-base/base/application/application.h"
#include "dfm-base/base/standardpaths.h"
#include "dfm-base/utils/dialogmanager.h"
#include "dfm-base/base/device/deviceproxymanager.h"
#include "dfm-base/base/device/deviceutils.h"

#include <dfm-framework/framework.h>

#include <QtConcurrent>
#include <QApplication>

#include <unistd.h>

DPCOMPUTER_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

bool ComputerUtils::contextMenuEnabled = true;

QUrl ComputerUtils::makeBlockDevUrl(const QString &id)
{
    QUrl devUrl;
    devUrl.setScheme(Global::kEntry);
    auto shortenBlk = id;
    shortenBlk.remove(QString(DeviceId::kBlockDeviceIdPrefix));   // /org/freedesktop/UDisks2/block_devices/sda1 -> sda1
    auto path = QString("%1.%2").arg(shortenBlk).arg(SuffixInfo::kBlock);   // sda1.blockdev
    devUrl.setPath(path);   // entry:sda1.blockdev
    return devUrl;
}

QString ComputerUtils::getBlockDevIdByUrl(const QUrl &url)
{
    if (url.scheme() != Global::kEntry)
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
    devUrl.setScheme(Global::kEntry);
    auto path = id.toUtf8().toBase64();
    QString encodecPath = QString("%1.%2").arg(QString(path)).arg(SuffixInfo::kProtocol);
    devUrl.setPath(encodecPath);
    return devUrl;
}

QString ComputerUtils::getProtocolDevIdByUrl(const QUrl &url)
{
    if (url.scheme() != Global::kEntry)
        return "";
    if (!url.path().endsWith(SuffixInfo::kProtocol))
        return "";

    QString suffix = QString(".%1").arg(SuffixInfo::kProtocol);
    QString encodecId = url.path().remove(suffix);
    QString id = QByteArray::fromBase64(encodecId.toUtf8());
    return id;
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
    url.setScheme(Global::kEntry);
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
    origUrl.setScheme(Global::kFile);
    origUrl.setPath(QString("%1/%2.%3").arg(StandardPaths::location(StandardPaths::kExtensionsAppEntryPath)).arg(fileName).arg("desktop"));
    return origUrl;
}

QUrl ComputerUtils::makeStashedProtocolDevUrl(const QString &id)
{
    QUrl devUrl;
    devUrl.setScheme(Global::kEntry);
    auto path = id.toUtf8().toBase64();
    QString encodecPath = QString("%1.%2").arg(QString(path)).arg(SuffixInfo::kStashedProtocol);
    devUrl.setPath(encodecPath);
    return devUrl;
}

QString ComputerUtils::getProtocolDevIdByStashedUrl(const QUrl &url)
{
    if (url.scheme() != Global::kEntry)
        return "";
    if (!url.path().endsWith(SuffixInfo::kStashedProtocol))
        return "";

    QString suffix = QString(".%1").arg(SuffixInfo::kStashedProtocol);
    QString encodecId = url.path().remove(suffix);
    QString id = QByteArray::fromBase64(encodecId.toUtf8());
    return id;
}

QUrl ComputerUtils::convertToProtocolDevUrlFrom(const QUrl &stashedUrl)
{
    if (stashedUrl.scheme() != Global::kEntry)
        return {};
    if (!stashedUrl.path().endsWith(SuffixInfo::kStashedProtocol))
        return {};

    QString path = stashedUrl.path();
    path.replace(SuffixInfo::kStashedProtocol, SuffixInfo::kProtocol);
    QUrl ret;
    ret.setScheme(Global::kEntry);
    ret.setPath(path);
    return ret;
}

QUrl ComputerUtils::convertToStashedUrlFrom(const QUrl &protocolDevUrl)
{
    if (protocolDevUrl.scheme() != Global::kEntry)
        return {};
    if (!protocolDevUrl.path().endsWith(SuffixInfo::kProtocol))
        return {};

    QString path = protocolDevUrl.path();
    path.replace(SuffixInfo::kProtocol, SuffixInfo::kStashedProtocol);
    QUrl ret;
    ret.setScheme(Global::kEntry);
    ret.setPath(path);
    return ret;
}

QUrl ComputerUtils::makeLocalUrl(const QString &path)
{
    QUrl u;
    u.setScheme(Global::kFile);
    u.setPath(path);
    return u;
}

QUrl ComputerUtils::makeBurnUrl(const QString &id)
{
    QString dev = id.mid(id.lastIndexOf("/") + 1);
    QUrl u;
    u.setScheme(Global::kBurn);
    // burn:///dev/sr0/disc_files/
    u.setPath(QString("/dev/%1/disc_files/").arg(dev));
    return u;
}

quint64 ComputerUtils::getWinId(QWidget *widget)
{
    auto &ctx = dpfInstance.serviceContext();
    auto winServ = ctx.service<DSB_FM_NAMESPACE::WindowsService>(DSB_FM_NAMESPACE::WindowsService::name());
    return winServ->findWindowId(widget);
}

bool ComputerUtils::isPresetSuffix(const QString &suffix)
{
    return suffix == SuffixInfo::kBlock || suffix == SuffixInfo::kProtocol || suffix == SuffixInfo::kUserDir
            || suffix == SuffixInfo::kAppEntry || suffix == SuffixInfo::kStashedProtocol;
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
    if (a.scheme() != Global::kEntry || b.scheme() != Global::kEntry)
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
    if (!url.path().startsWith(QString("/run/user/%1/gvfs/").arg(getuid())))
        return true;
    setCursorState(true);

    std::string path = url.path().toStdString();
    bool exist = false;
    auto &&fu = QtConcurrent::run([path, &exist, timeout] {
        QTime t;
        t.start();
        int ret = access(path.c_str(), F_OK);
        qDebug() << "gvfs path: " << path.c_str() << ", exist: " << (ret == 0) << ", error: " << strerror(ret);
        if (t.elapsed() < timeout) {
            exist = (ret == 0);
            condForCheckGvfs.wakeAll();
        }
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

QString ComputerUtils::deviceTypeInfo(DFMEntryFileInfoPointer info)
{
    DFMBASE_USE_NAMESPACE
    switch (info->order()) {
    case EntryFileInfo::kOrderUserDir:
        return QObject::tr("User directory");
    case EntryFileInfo::kOrderSysDiskRoot:
    case EntryFileInfo::kOrderSysDiskData:
    case EntryFileInfo::kOrderSysDisks:
        return QObject::tr("Local disk");
    case EntryFileInfo::kOrderRemovableDisks:
        return QObject::tr("Removable disk");
    case EntryFileInfo::kOrderOptical:
        return QObject::tr("DVD");
    case EntryFileInfo::kOrderSmb:
    case EntryFileInfo::kOrderFtp:
        if (info->suffix() == SuffixInfo::kStashedProtocol)
            return QObject::tr("Unconnected network shared directory");
        return QObject::tr("Network shared directory");
    case EntryFileInfo::kOrderMTP:
        return QObject::tr("Android mobile device");
    case EntryFileInfo::kOrderGPhoto2:
        if (getProtocolDevIdByUrl(info->url()).contains("Apple_Inc"))
            return QObject::tr("Apple mobile device");
        return QObject::tr("Android mobile device");
    case EntryFileInfo::kOrderFiles:
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
    DSC_NAMESPACE::Property::DeviceInfo devInfo;
    devInfo.icon = info->fileIcon();
    devInfo.deviceUrl = info->url();
    devInfo.mountPoint = info->targetUrl();
    devInfo.deviceName = info->displayName();
    devInfo.deviceType = ComputerUtils::deviceTypeInfo(info);
    devInfo.fileSystem = info->extraProperty(GlobalServerDefines::DeviceProperty::kFileSystem).toString();
    devInfo.totalCapacity = info->sizeTotal();
    devInfo.availableSpace = info->sizeFree();
    dialog->setSelectDeviceInfo(devInfo);
    return dialog;
}

QUrl ComputerUtils::convertToDevUrl(const QUrl &url)
{
    if (url.scheme() == Global::kEntry)
        return url;

    QUrl converted = url;
    if (delegateServIns->isRegistedTransparentHandle(converted.scheme()))   // first convert it to local file
        converted = delegateServIns->urlTransform(converted);

    QString devId;
    if (converted.scheme() == Global::kFile && DevProxyMng->isMptOfDevice(converted.path(), devId)) {
        if (devId.startsWith(kBlockDeviceIdPrefix))
            converted = ComputerUtils::makeBlockDevUrl(devId);
        else
            converted = ComputerUtils::makeProtocolDevUrl(devId);
    } else if (!converted.isValid() && url.scheme() == Global::kBurn) {
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

    qDebug() << "convert url from" << url << "to" << converted;
    return converted;
}
