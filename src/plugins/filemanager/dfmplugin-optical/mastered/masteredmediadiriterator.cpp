// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "masteredmediadiriterator.h"
#include "masteredmediafileinfo.h"

#include "utils/opticalhelper.h"

#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/dbusservice/global_server_defines.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/asyncfileinfo.h>

#include <QStandardPaths>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_optical;

using namespace GlobalServerDefines;

using namespace dfmio;

MasteredMediaDirIterator::MasteredMediaDirIterator(const QUrl &url,
                                                   const QStringList &nameFilters,
                                                   QDir::Filters filters,
                                                   QDirIterator::IteratorFlags flags)
    : AbstractDirIterator(url, nameFilters, filters, flags)
{
    devFile = OpticalHelper::burnDestDevice(url);
    QString id { DeviceUtils::getBlockDeviceId(devFile) };
    auto &&map = DevProxyMng->queryBlockInfo(id);
    mntPoint = qvariant_cast<QString>(map[DeviceProperty::kMountPoint]);
    fmDebug() << "Mount point for device" << devFile << "is:" << mntPoint;

    QString stagingPath { OpticalHelper::localStagingFile(url).path() };
    OpticalHelper::createStagingFolder(devFile);

    stagingIterator.reset(new dfmio::DEnumerator(QUrl::fromLocalFile(stagingPath), nameFilters,
                                                 static_cast<DEnumerator::DirFilter>(static_cast<int32_t>(filters)),
                                                 static_cast<DEnumerator::IteratorFlag>(static_cast<uint8_t>(flags))));

    bool opticalBlank { qvariant_cast<bool>(map[DeviceProperty::kOpticalBlank]) };
    if (opticalBlank) {
        fmInfo() << "Optical disc is blank, using staging iterator only for device:" << devFile;
        discIterator.clear();
        return;
    }
    QString realpath { mntPoint + OpticalHelper::burnFilePath(url) };
    if (realpath != "/")
        discIterator.reset(new dfmio::DEnumerator(QUrl::fromLocalFile(realpath), nameFilters,
                                                  static_cast<DEnumerator::DirFilter>(static_cast<int32_t>(filters)),
                                                  static_cast<DEnumerator::IteratorFlag>(static_cast<uint8_t>(flags))));
}

QUrl MasteredMediaDirIterator::next()
{
    if (discIterator) {
        currentUrl = discIterator->next();
    } else {
        discIterator = nullptr;
        currentUrl = stagingIterator->next();
    }
    return changeScheme(currentUrl);
}

bool MasteredMediaDirIterator::hasNext() const
{
    // hasnext 会跳转到下一个
    if (discIterator && discIterator->hasNext())
        return true;
    if (discIterator)
        discIterator.clear();
    return stagingIterator && stagingIterator->hasNext();
}

QString MasteredMediaDirIterator::fileName() const
{
    return fileUrl().fileName();
}

QUrl MasteredMediaDirIterator::fileUrl() const
{
    return changeScheme(currentUrl);
}

const FileInfoPointer MasteredMediaDirIterator::fileInfo() const
{
    assert(QThread::currentThread() != qApp->thread());

    QSharedPointer<DFileInfo> dfmfileinfo = discIterator ? discIterator->fileInfo() : stagingIterator->fileInfo();

    auto url = currentUrl;
    if (!url.isValid() || dfmfileinfo.isNull()) {
        //bug 64941, DVD+RW 只擦除文件系统部分信息，而未擦除全部，有垃圾数据，所以需要判断文件的有效性
        FileInfoPointer fileinfo = FileInfoPointer(new MasteredMediaFileInfo(fileUrl()));
        return fileinfo->exists() ? fileinfo : FileInfoPointer();
    }

    const QString &fileName = dfmfileinfo->attribute(DFileInfo::AttributeID::kStandardName, nullptr).toString();
    bool isHidden = false;
    if (fileName.startsWith(".")) {
        isHidden = true;
    }

    QSharedPointer<FileInfo> info = QSharedPointer<AsyncFileInfo>(new AsyncFileInfo(url, dfmfileinfo));
    info.dynamicCast<AsyncFileInfo>()->cacheAsyncAttributes();
    info->setExtendedAttributes(ExtInfoType::kFileIsHid, isHidden);

    QSharedPointer<FileInfo> infoTrans = InfoFactory::transfromInfo<FileInfo>(url.scheme(), info);
    if (infoTrans) {
        infoTrans->setExtendedAttributes(ExtInfoType::kFileIsHid, isHidden);
        infoTrans->setExtendedAttributes(ExtInfoType::kFileLocalDevice, false);
        infoTrans->setExtendedAttributes(ExtInfoType::kFileCdRomDevice, false);
    } else {
        fmWarning() << "MasteredMediaFileInfo: info is nullptr, url = " << url;
        //bug 64941, DVD+RW 只擦除文件系统部分信息，而未擦除全部，有垃圾数据，所以需要判断文件的有效性
        FileInfoPointer fileinfo = FileInfoPointer(new MasteredMediaFileInfo(fileUrl()));
        return fileinfo->exists() ? fileinfo : FileInfoPointer();
    }

    //bug 64941, DVD+RW 只擦除文件系统部分信息，而未擦除全部，有垃圾数据，所以需要判断文件的有效性
    FileInfoPointer fileinfo = FileInfoPointer(new MasteredMediaFileInfo(fileUrl(), infoTrans));
    return fileinfo->exists() ? fileinfo : FileInfoPointer();
}

QUrl MasteredMediaDirIterator::url() const
{
    return changeScheme(discIterator ? discIterator->uri() : stagingIterator->uri());
}

QUrl MasteredMediaDirIterator::changeScheme(const QUrl &in) const
{
    QUrl burntmp = QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation)
                                       + "/" + qApp->organizationName() + "/" + QString(kDiscburnStaging) + "/");
    QString stagingroot = burntmp.path() + QString(devFile).replace('/', '_');
    QUrl ret;
    QString path = in.path();
    if (burntmp.isParentOf(in)) {
        path.replace(stagingroot, devFile + "/" + kBurnSegStaging);
    } else {
        path.replace(mntPoint, devFile + "/" + kBurnSegOndisc);
    }
    ret.setScheme(Global::Scheme::kBurn);
    ret.setPath(path);

    return ret;
}
