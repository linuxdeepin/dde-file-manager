// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "entryfileinfo.h"
#include "private/entryfileinfo_p.h"
#include "dbusservice/global_server_defines.h"
#include "base/standardpaths.h"
#include <dfm-base/dfm_global_defines.h>

#include <QRegularExpression>
#include <QAction>

namespace dfmbase {

EntryFileInfoPrivate::EntryFileInfoPrivate(EntryFileInfo *qq)
    : FileInfoPrivate(qq)
{
}

void EntryFileInfoPrivate::init()
{
    const auto &&suffix = const_cast<EntryFileInfoPrivate *>(this)->suffix();
    entity.reset(EntryEntityFactor::create(suffix, q->urlOf(UrlInfoType::kUrl)));
}

QString EntryFileInfoPrivate::suffix() const
{
    QRegularExpression re(".*\\.(.*)$");
    auto rem = re.match(q->fileUrl().path());
    if (!rem.hasMatch()) {
        return "";
    }
    return rem.captured(1);
}

EntryFileInfoPrivate::~EntryFileInfoPrivate()
{
}

/*!
 * \class EntryFileInfo
 * \brief this is a proxy class for file info, the real info is in private class and should be registered by suffix
 */
EntryFileInfo::EntryFileInfo(const QUrl &url)
    : FileInfo(url), d(new EntryFileInfoPrivate(this))
{
    d->init();
    Q_ASSERT_X(url.scheme() == Global::Scheme::kEntry, __FUNCTION__, "This is not EntryFileInfo's scheme");
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

quint64 EntryFileInfo::sizeTotal() const
{
    return d->entity ? d->entity->sizeTotal() : 0;
}

quint64 EntryFileInfo::sizeUsage() const
{
    return d->entity ? d->entity->sizeUsage() : 0;
}

quint64 EntryFileInfo::sizeFree() const
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

QString EntryFileInfo::nameOf(const NameInfoType type) const
{
    switch (type) {
    case NameInfoType::kBaseName:
        return {};
    case NameInfoType::kSuffix:
        return d->suffix();
    default:
        return FileInfo::nameOf(type);
    }
}

QString EntryFileInfo::pathOf(const PathInfoType type) const
{
    QString path;
    switch (type) {
    case FilePathInfoType::kPath:
        [[fallthrough]];
    case FilePathInfoType::kFilePath:
        return url.path();
    default:
        return FileInfo::pathOf(type);
    }
}

QIcon EntryFileInfo::fileIcon()
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

}
