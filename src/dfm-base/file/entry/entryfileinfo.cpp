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
const char *const kStashedRemote { "stashedprotodev" };
}   // namespace SuffixInfo

EntryFileInfoPrivate::EntryFileInfoPrivate(EntryFileInfo *qq)
    : AbstractFileInfoPrivate(qq), q(qq)
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
    : AbstractFileInfo(url), d(new EntryFileInfoPrivate(this))
{
    Q_ASSERT_X(url.scheme() == SchemeTypes::kEntry, __FUNCTION__, "This is not EntryFileInfo's scheme");
}

EntryFileInfo::~EntryFileInfo()
{
}

EntryFileInfo::EntryOrder EntryFileInfo::order() const
{
    return d->entity ? d->entity->order() : EntryOrder::kOrderFiles;
}

bool EntryFileInfo::removable() const
{
    return d->entity ? d->entity->removable() : false;
}

QUrl EntryFileInfo::targetUrl() const
{
    return d->entity ? d->entity->targetUrl() : QUrl();
}

QMenu *EntryFileInfo::createMenu() const
{
    return d->entity ? d->entity->createMenu() : nullptr;
}

bool EntryFileInfo::isEncrypted() const
{
    return d->entity ? d->entity->isEncrypted() : false;
}

bool EntryFileInfo::isUnlocked() const
{
    return d->entity ? d->entity->isUnlocked() : false;
}

QString EntryFileInfo::clearDeviceId() const
{
    return d->entity ? d->entity->clearDeviceId() : "";
}

bool EntryFileInfo::renamable() const
{
    // TODO(xust)
    return false;
}

QString EntryFileInfo::displayName() const
{
    return d->entity ? d->entity->displayName() : "";
}

long EntryFileInfo::sizeTotal() const
{
    return d->entity ? d->entity->sizeTotal() : 0;
}

long EntryFileInfo::sizeUsage() const
{
    return d->entity ? d->entity->sizeUsage() : 0;
}

long EntryFileInfo::sizeFree() const
{
    return d->entity ? sizeTotal() - sizeUsage() : 0;
}

QString EntryFileInfo::fileSystem() const
{
    return d->entity ? d->entity->fileSystem() : "";
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

DFMBASE_END_NAMESPACE
