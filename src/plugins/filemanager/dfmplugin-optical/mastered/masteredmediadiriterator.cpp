/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#include "masteredmediadiriterator.h"
#include "masteredmediafileinfo.h"

#include "utils/opticalhelper.h"

#include "dfm-base/utils/devicemanager.h"
#include "dfm-base/dbusservice/global_server_defines.h"

DFMBASE_USE_NAMESPACE
DPOPTICAL_USE_NAMESPACE

using namespace GlobalServerDefines;

MasteredMediaDirIterator::MasteredMediaDirIterator(const QUrl &url,
                                                   const QStringList &nameFilters,
                                                   QDir::Filters filters,
                                                   QDirIterator::IteratorFlags flags)
    : AbstractDirIterator(url, nameFilters, filters, flags)
{
    devFile = OpticalHelper::burnDestDevice(url);
    QString id { DeviceManager::blockDeviceId(devFile) };
    auto &&map = DeviceManagerInstance.invokeQueryBlockDeviceInfo(id);
    mntPoint = qvariant_cast<QString>(map[DeviceProperty::kMountPoint]);

    QString stagingPath { OpticalHelper::localStagingFile(url).path() };
    stagingIterator = QSharedPointer<QDirIterator>(
            new QDirIterator(stagingPath, nameFilters, filters, flags));

    bool opticalBlank { qvariant_cast<bool>(map[DeviceProperty::kOpticalBlank]) };
    if (opticalBlank) {
        discIterator.clear();
        return;
    }

    QString realpath { mntPoint + OpticalHelper::burnFilePath(url) };
    discIterator = QSharedPointer<QDirIterator>(new QDirIterator(realpath, nameFilters, filters, flags));
}

QUrl MasteredMediaDirIterator::next()
{
    if (discIterator && discIterator->hasNext()) {
        return changeSchemeUpdate(discIterator->next());
    } else {
        discIterator = nullptr;
        return changeSchemeUpdate(stagingIterator->next());
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

const AbstractFileInfoPointer MasteredMediaDirIterator::fileInfo() const
{
    AbstractFileInfoPointer fileinfo = AbstractFileInfoPointer(new MasteredMediaFileInfo(fileUrl()));
    return fileinfo->exists() ? fileinfo : AbstractFileInfoPointer();   //bug 64941, DVD+RW 只擦除文件系统部分信息，而未擦除全部，有垃圾数据，所以需要判断文件的有效性
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
    ret.setScheme(SchemeTypes::kBurn);
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
