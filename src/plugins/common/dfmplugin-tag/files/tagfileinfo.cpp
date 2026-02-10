// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tagfileinfo.h"
#include "utils/tagmanager.h"
#include "tagfileinfo_p.h"

#include <dfm-base/interfaces/private/fileinfo_p.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/fileutils.h>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_tag;

TagFileInfo::TagFileInfo(const QUrl &url)
    : ProxyFileInfo(url), d(new TagFileInfoPrivate(this))
{
}

TagFileInfo::~TagFileInfo()
{
}

bool TagFileInfo::exists() const
{
    QUrl rootUrl;
    rootUrl.setScheme(TagManager::scheme());
    if (urlOf(UrlInfoType::kUrl) == rootUrl)
        return true;

    const QMap<QString, QColor> &tagMap = TagManager::instance()->getAllTags();

    return tagMap.contains(tagName());
}

QFileDevice::Permissions TagFileInfo::permissions() const
{
    return QFile::ReadGroup | QFile::ReadOwner | QFile::ReadUser | QFile::ReadOther
            | QFile::WriteGroup | QFile::WriteOwner | QFile::WriteUser | QFile::WriteOther;
}

bool TagFileInfo::isAttributes(const OptInfoType type) const
{
    switch (type) {
    case FileIsType::kIsReadable:
        return true;
    case FileIsType::kIsWritable:
        return true;
    case FileIsType::kIsExecutable:
        return true;
    default:
        return ProxyFileInfo::isAttributes(type);
    }
}

bool TagFileInfo::canAttributes(const CanableInfoType type) const
{
    switch (type) {
    case FileCanType::kCanDrop:
        return true;
    default:
        return ProxyFileInfo::canAttributes(type);
    }
}

QString TagFileInfo::nameOf(const NameInfoType type) const
{
    switch (type) {
    case NameInfoType::kFileName:
    case NameInfoType::kFileCopyName:
        return d->fileName();
    default:
        return ProxyFileInfo::nameOf(type);
    }
}

QString TagFileInfo::displayOf(const DisPlayInfoType type) const
{
    if (DisPlayInfoType::kFileDisplayName == type)
        return d->fileName();
    return ProxyFileInfo::displayOf(type);
}

FileInfo::FileType TagFileInfo::fileType() const
{
    return FileType::kDirectory;
}

QIcon TagFileInfo::fileIcon()
{
    return QIcon::fromTheme("folder");
}

QString TagFileInfo::tagName() const
{
    return d->fileName();
}

TagFileInfoPrivate::TagFileInfoPrivate(TagFileInfo *qq)
    : q(qq)
{
}

TagFileInfoPrivate::~TagFileInfoPrivate()
{
}

QString TagFileInfoPrivate::fileName() const
{
    return q->fileUrl().path().mid(1, q->fileUrl().path().length() - 1);
}
