// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "masteredmediadiriterator.h"
#include "masteredmediafileinfo.h"

#include "utils/opticalhelper.h"

#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/dbusservice/global_server_defines.h>

#include <QStandardPaths>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_optical;

using namespace GlobalServerDefines;

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

    QString stagingPath { OpticalHelper::localStagingFile(url).path() };
    OpticalHelper::createStagingFolder(devFile);

    stagingIterator = QSharedPointer<QDirIterator>(
            new QDirIterator(stagingPath, nameFilters, filters, flags));

    bool opticalBlank { qvariant_cast<bool>(map[DeviceProperty::kOpticalBlank]) };
    if (opticalBlank) {
        discIterator.clear();
        return;
    }

    QString realpath { mntPoint + OpticalHelper::burnFilePath(url) };
    if (realpath != "/")
        discIterator = QSharedPointer<QDirIterator>(new QDirIterator(realpath, nameFilters, filters, flags));
}

QUrl MasteredMediaDirIterator::next()
{
    if (discIterator && discIterator->hasNext()) {
        return changeSchemeUpdate(QUrl::fromLocalFile(discIterator->next()));
    } else {
        discIterator = nullptr;
        return changeSchemeUpdate(QUrl::fromLocalFile(stagingIterator->next()));
    }
}

bool MasteredMediaDirIterator::hasNext() const
{
    return (discIterator && discIterator->hasNext()) || (stagingIterator && stagingIterator->hasNext());
}

QString MasteredMediaDirIterator::fileName() const
{
    return discIterator ? discIterator->fileName() : stagingIterator->fileName();
}

QUrl MasteredMediaDirIterator::fileUrl() const
{
    return changeScheme(QUrl::fromLocalFile(discIterator ? discIterator->filePath() : stagingIterator->filePath()));
}

const FileInfoPointer MasteredMediaDirIterator::fileInfo() const
{
    FileInfoPointer fileinfo = FileInfoPointer(new MasteredMediaFileInfo(fileUrl()));
    return fileinfo->exists() ? fileinfo : FileInfoPointer();   //bug 64941, DVD+RW 只擦除文件系统部分信息，而未擦除全部，有垃圾数据，所以需要判断文件的有效性
}

QUrl MasteredMediaDirIterator::url() const
{
    return changeScheme(QUrl::fromLocalFile(discIterator ? discIterator->path() : stagingIterator->path()));
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
    if (skip.contains(ret)) {
        ret.setFragment("dup");
    }
    return ret;
}

QUrl MasteredMediaDirIterator::changeSchemeUpdate(const QUrl &in)
{
    QUrl ret = changeScheme(in);
    if (seen.contains(OpticalHelper::burnFilePath(ret))) {
        skip.insert(ret);
        return QUrl();
    }
    seen.insert(OpticalHelper::burnFilePath(ret));
    return ret;
}
