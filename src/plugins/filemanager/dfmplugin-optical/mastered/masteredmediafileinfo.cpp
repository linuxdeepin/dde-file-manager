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
#include "dfm-base/interfaces/private/abstractfileinfo_p.h"
#include "dfm-base/utils/devicemanager.h"
#include "dfm-base/dbusservice/global_server_defines.h"

DFMBASE_USE_NAMESPACE

DPOPTICAL_BEGIN_NAMESPACE

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
    if (!OpticalHelper::burnIsOnDisc(backerUrl))
        return true;
    QString id { DeviceManager::blockDeviceId(OpticalHelper::burnDestDevice(url())) };
    auto &&map = DeviceManagerInstance.invokeQueryBlockDeviceInfo(id);
    quint64 avil { qvariant_cast<quint64>(map[DeviceProperty::kSizeFree]) };
    return avil > 0;
}

bool MasteredMediaFileInfo::isDir() const
{
    return !dptr->proxy || dptr->proxy->isDir();
}

QString MasteredMediaFileInfo::fileDisplayName() const
{
    if (OpticalHelper::burnFilePath(url()).contains(QRegularExpression("^(/*)$"))) {
        QString id { DeviceManager::blockDeviceId(OpticalHelper::burnDestDevice(url())) };
        auto &&map = DeviceManagerInstance.invokeQueryBlockDeviceInfo(id);
        QString idLabel { qvariant_cast<QString>(map[DeviceProperty::kIdLabel]) };
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

QVector<ActionType> MasteredMediaFileInfo::menuActionList(AbstractMenu::MenuType type) const
{
    // TODO(zhangs):
    return {};
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

bool MasteredMediaFileInfo::canDrop() const
{
    return isWritable();
}

bool MasteredMediaFileInfo::canRename() const
{
    return false;
}

QSet<ActionType> MasteredMediaFileInfo::disableMenuActionList() const
{
    // TODO(zhangs):
    return {};
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

QString MasteredMediaFileInfo::emptyDirectoryTip() const
{
    return QObject::tr("Folder is empty");
}

void MasteredMediaFileInfo::backupInfo(const QUrl &url)
{
    if (OpticalHelper::burnDestDevice(url).length() == 0)
        return;

    if (OpticalHelper::burnIsOnDisc(url)) {
        QString id { DeviceManager::blockDeviceId(OpticalHelper::burnDestDevice(url)) };
        auto &&map = DeviceManagerInstance.invokeQueryBlockDeviceInfo(id);
        bool opticalBlank { qvariant_cast<bool>(map[DeviceProperty::kOpticalBlank]) };
        if (opticalBlank)
            return;
        QString mnt { qvariant_cast<QString>(map[DeviceProperty::kMountPoint]) };
        if (mnt.isEmpty())
            backerUrl = QUrl();
        else
            backerUrl = QUrl::fromLocalFile(mnt + OpticalHelper::burnFilePath(url));
    } else {
        backerUrl = OpticalHelper::localStagingFile(url);
    }
    setProxy(InfoFactory::create<AbstractFileInfo>(backerUrl));
}

DPOPTICAL_END_NAMESPACE
