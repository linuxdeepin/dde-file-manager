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
#include "entryfileinfo.h"
#include "private/entryfileinfo_p.h"
#include "dbusservice/global_server_defines.h"
#include "base/standardpaths.h"

#include <QRegularExpression>
#include <QAction>

DFMBASE_BEGIN_NAMESPACE

namespace DeviceId {
const char *const kBlockDeviceIdPrefix { "/org/freedesktop/UDisks2/block_devices/" };
}   // namespace DeviceId

namespace SuffixInfo {
const char *const kUserDir { "userdir" };
const char *const kProtocol { "protodev" };
const char *const kBlock { "blockdev" };
}   // namespace SuffixInfo

// this might be a temperary solution
namespace ContextMenuActionTrs {
QString trOpenInNewWin()
{
    return QObject::tr("Open in new window");
}
QString trOpenInNewTab()
{
    return QObject::tr("Open in new tab");
}
QString trMount()
{
    return QObject::tr("Mount");
}
QString trUnmount()
{
    return QObject::tr("Unmount");
}
QString trRename()
{
    return QObject::tr("Rename");
}
QString trEject()
{
    return QObject::tr("Eject");
}
QString trSafelyRemove()
{
    return QObject::tr("Safely Remove");
}
QString trProperties()
{
    return QObject::tr("Properties");
}
QString trFormat()
{
    return QObject::tr("Format");
}
QString trRemove()
{
    return QObject::tr("Remove");
}
QString trLogoutAndClearSavedPasswd()
{
    return QObject::tr("Clear saved password and unmount");
}
}   // namespace ContextMenuActionTrs

EntryFileInfoPrivate::EntryFileInfoPrivate(EntryFileInfo *qq)
    : AbstractFileInfoPrivate(qq)
{
}

void EntryFileInfoPrivate::init()
{
    const auto &&suffix = q->suffix();
    entity.reset(EntryEntityFactor::create(suffix, q->url()));
}

EntryFileInfoPrivate::~EntryFileInfoPrivate()
{
}

/*!
 * \class EntryFileInfo
 * \brief this is a proxy class for file info, the real info is in private class and should be registered by suffix
 */
EntryFileInfo::EntryFileInfo(const QUrl &url)
    : AbstractFileInfo(url, new EntryFileInfoPrivate(this))
{
    d = static_cast<EntryFileInfoPrivate *>(dptr.data());
    d->init();
    Q_ASSERT_X(url.scheme() == SchemeTypes::kEntry, __FUNCTION__, "This is not EntryFileInfo's scheme");
}

EntryFileInfo::~EntryFileInfo()
{
    d = nullptr;
}

EntryFileInfo::EntryOrder EntryFileInfo::order() const
{
    return d->entity ? d->entity->order() : EntryOrder::kOrderCustom;
}

QUrl EntryFileInfo::targetUrl() const
{
    return d->entity ? d->entity->targetUrl() : QUrl();
}

QMenu *EntryFileInfo::createMenu() const
{
    return d->entity ? d->entity->createMenu() : nullptr;
}

bool EntryFileInfo::isAccessable() const
{
    return d->entity ? d->entity->isAccessable() : false;
}

QString EntryFileInfo::description() const
{
    return d->entity ? d->entity->description() : "";
}

QVariant EntryFileInfo::extraProperty(const QString &property) const
{
    auto properties = extraProperties();
    return properties.contains(property) ? properties[property] : QVariant();
}

void EntryFileInfo::setExtraProperty(const QString &property, const QVariant &value)
{
    if (d->entity)
        d->entity->setExtraProperty(property, value);
}

bool EntryFileInfo::renamable() const
{
    return d->entity ? d->entity->renamable() : false;
}

QString EntryFileInfo::displayName() const
{
    return d->entity ? d->entity->displayName() : "";
}

qint64 EntryFileInfo::sizeTotal() const
{
    return d->entity ? d->entity->sizeTotal() : 0;
}

qint64 EntryFileInfo::sizeUsage() const
{
    return d->entity ? d->entity->sizeUsage() : 0;
}

qint64 EntryFileInfo::sizeFree() const
{
    return d->entity ? sizeTotal() - sizeUsage() : 0;
}

bool EntryFileInfo::showTotalSize() const
{
    return d->entity ? d->entity->showTotalSize() : false;
}

bool EntryFileInfo::showUsedSize() const
{
    return d->entity ? d->entity->showUsageSize() : false;
}

bool EntryFileInfo::showProgress() const
{
    return d->entity ? d->entity->showProgress() : false;
}

bool EntryFileInfo::exists() const
{
    return d->entity ? d->entity->exists() : false;
}

QString EntryFileInfo::filePath() const
{
    return path();
}

QString EntryFileInfo::baseName() const
{
    return {};
}

QString EntryFileInfo::suffix() const
{
    QRegularExpression re(".*\\.(.*)$");
    auto rem = re.match(path());
    if (!rem.hasMatch()) {
        return "";
    }
    return rem.captured(1);
}

QString EntryFileInfo::path() const
{
    return url().path();
}

QIcon EntryFileInfo::fileIcon() const
{
    return d->entity ? d->entity->icon() : QIcon();
}

void EntryFileInfo::refresh()
{
    if (d->entity)
        d->entity->refresh();
}

QVariantHash EntryFileInfo::extraProperties() const
{
    return d->entity ? d->entity->extraProperties() : QVariantHash();
}

DFMBASE_END_NAMESPACE
