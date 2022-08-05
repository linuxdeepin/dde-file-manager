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
#include "masteredmediafileinfo.h"

#include "utils/opticalhelper.h"

#include "dfm-base/base/schemefactory.h"
#include "dfm-base/base/device/deviceproxymanager.h"
#include "dfm-base/base/device/deviceutils.h"
#include "dfm-base/interfaces/private/abstractfileinfo_p.h"
#include "dfm-base/dbusservice/global_server_defines.h"

DFMBASE_USE_NAMESPACE

namespace dfmplugin_optical {

using namespace GlobalServerDefines;

class MasteredMediaFileInfoPrivate : public AbstractFileInfoPrivate
{
public:
    explicit MasteredMediaFileInfoPrivate(AbstractFileInfo *qq)
        : AbstractFileInfoPrivate(qq)
    {
    }

    virtual ~MasteredMediaFileInfoPrivate();
};
MasteredMediaFileInfoPrivate::~MasteredMediaFileInfoPrivate()
{
}

MasteredMediaFileInfo::MasteredMediaFileInfo(const QUrl &url)
    : AbstractFileInfo(url, new MasteredMediaFileInfoPrivate(this))
{
    backupInfo(url);
}

bool MasteredMediaFileInfo::exists() const
{
    if (url().isEmpty() || !backerUrl.isValid() || backerUrl.isEmpty()) {
        return false;
    }
    if (url().fragment() == "dup") {
        return false;
    }

    return dptr->proxy && dptr->proxy->exists();
}

bool MasteredMediaFileInfo::isReadable() const
{
    if (!dptr->proxy)
        return true;

    return dptr->proxy->isReadable();
}

bool MasteredMediaFileInfo::isWritable() const
{
    if (!dptr->proxy)
        return false;
    return dptr->proxy->isWritable();
}

bool MasteredMediaFileInfo::isDir() const
{
    return !dptr->proxy || dptr->proxy->isDir();
}

QString MasteredMediaFileInfo::fileDisplayName() const
{
    if (OpticalHelper::burnFilePath(url()).contains(QRegularExpression("^(/*)$"))) {
        QString id { DeviceUtils::getBlockDeviceId(OpticalHelper::burnDestDevice(url())) };
        QString idLabel { qvariant_cast<QString>(devInfoMap[DeviceProperty::kIdLabel]) };
        return idLabel;
    }

    if (!dptr->proxy)
        return "";
    return dptr->proxy->fileDisplayName();
}

QVariantHash MasteredMediaFileInfo::extraProperties() const
{
    QVariantHash ret;
    if (dptr->proxy) {
        ret = dptr->proxy->extraProperties();
    }
    ret["mm_backer"] = backerUrl.path();
    return ret;
}

bool MasteredMediaFileInfo::canRedirectionFileUrl() const
{
    if (isDir())
        return isSymLink();   // fix bug 202007010021 当光驱刻录的文件夹中存在文件夹的链接时，要跳转到链接对应的目标文件夹
    return !isDir();
}

QUrl MasteredMediaFileInfo::redirectedFileUrl() const
{
    if (dptr->proxy) {
        return dptr->proxy->url();
    }

    return AbstractFileInfo::url();
}

QUrl MasteredMediaFileInfo::parentUrl() const
{
    QString burnPath { OpticalHelper::burnFilePath(url()) };
    if (burnPath.contains(QRegularExpression("^(/*)$"))) {
        return QUrl::fromLocalFile(QDir::homePath());
    }
    return UrlRoute::urlParent(url());
}

bool MasteredMediaFileInfo::canDrop()
{
    if (!OpticalHelper::burnIsOnDisc(backerUrl))
        return true;
    QString id { DeviceUtils::getBlockDeviceId(OpticalHelper::burnDestDevice(url())) };
    quint64 avil { qvariant_cast<quint64>(devInfoMap[DeviceProperty::kSizeFree]) };
    return avil > 0;
}

bool MasteredMediaFileInfo::canRename() const
{
    if (dptr->proxy)
        return dptr->proxy->canRename();

    return false;
}

void MasteredMediaFileInfo::refresh()
{
    AbstractFileInfo::refresh();
    if (dptr->proxy) {
        return;
    }

    backupInfo(url());
}

bool MasteredMediaFileInfo::canDragCompress() const
{
    return false;
}

bool MasteredMediaFileInfo::canHidden() const
{
    return false;
}

bool MasteredMediaFileInfo::canTag() const
{
    return false;
}

QString MasteredMediaFileInfo::emptyDirectoryTip() const
{
    return QObject::tr("Folder is empty");
}

Qt::DropActions MasteredMediaFileInfo::supportedDropActions()
{
    if (!OpticalHelper::isBurnEnabled())
        return Qt::IgnoreAction;
    return AbstractFileInfo::supportedDropActions();
}

void MasteredMediaFileInfo::backupInfo(const QUrl &url)
{
    if (OpticalHelper::burnDestDevice(url).length() == 0)
        return;

    if (OpticalHelper::burnIsOnDisc(url)) {
        QString &&devFile { OpticalHelper::burnDestDevice(url) };
        QString &&mnt { DeviceUtils::getMountInfo(devFile) };
        QString id { DeviceUtils::getBlockDeviceId(OpticalHelper::burnDestDevice(url)) };
        devInfoMap = DevProxyMng->queryBlockInfo(id);
        if (mnt.isEmpty())
            return;
        backerUrl = QUrl::fromLocalFile(mnt + OpticalHelper::burnFilePath(url));
    } else {
        backerUrl = OpticalHelper::localStagingFile(url);
    }
    setProxy(InfoFactory::create<AbstractFileInfo>(backerUrl));
}

}
