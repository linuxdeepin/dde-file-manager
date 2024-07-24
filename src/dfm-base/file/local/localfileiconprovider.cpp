// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "localfileiconprovider.h"

#include <dfm-io/dfileinfo.h>

namespace dfmbase {
class LocalFileIconProviderPrivate
{
public:
    LocalFileIconProviderPrivate();

    QIcon fileSystemIcon(const QString &path) const;
    QIcon fromTheme(QString iconName) const;
};
}

using namespace dfmbase;
LocalFileIconProviderPrivate::LocalFileIconProviderPrivate()
{
}

// fallback to gio
QIcon LocalFileIconProviderPrivate::fileSystemIcon(const QString &path) const
{
    QIcon icon;

    DFMIO::DFileInfo info(path);
    info.initQuerier();
    if (!info.exists())
        return icon;

    const QStringList &iconNames = info.attribute(DFMIO::DFileInfo::AttributeID::kStandardIcon).toStringList();
    if (!iconNames.isEmpty())
        icon = QIcon::fromTheme(iconNames.first());

    return icon;
}

QIcon LocalFileIconProviderPrivate::fromTheme(QString iconName) const
{
    assert(QThread::currentThread() == qApp->thread());
    QIcon icon;
    icon = QIcon::fromTheme(iconName);

    if (Q_LIKELY(!icon.isNull()))
        return icon;

    if (iconName == "application-vnd.debian.binary-package") {
        iconName = "application-x-deb";
    } else if (iconName == "application-vnd.rar") {
        iconName = "application-zip";
    } else if (iconName == "application-vnd.ms-htmlhelp") {
        iconName = "chmsee";
    } else if (iconName == "Zoom.png") {
        iconName = "application-x-zoom";
    } else {
        return icon;
    }

    icon = QIcon::fromTheme(iconName);

    return icon;
}

Q_GLOBAL_STATIC(LocalFileIconProvider, globalFIP)

LocalFileIconProvider::LocalFileIconProvider()
    : d(new LocalFileIconProviderPrivate())
{
}

LocalFileIconProvider::~LocalFileIconProvider()
{
}

LocalFileIconProvider *LocalFileIconProvider::globalProvider()
{
    return globalFIP;
}

QIcon LocalFileIconProvider::icon(const QFileInfo &info) const
{
    return d->fileSystemIcon(info.path());
}

QIcon LocalFileIconProvider::icon(const QString &path) const
{
    return d->fileSystemIcon(path);
}

QIcon LocalFileIconProvider::icon(const QFileInfo &info, const QIcon &feedback) const
{
    const QIcon &icon = this->icon(info);

    if (icon.isNull())
        return feedback;

    return icon;
}

QIcon LocalFileIconProvider::icon(const QString &path, const QIcon &feedback) const
{
    const QIcon &icon = this->icon(path);

    if (icon.isNull())
        return feedback;

    return icon;
}

QIcon LocalFileIconProvider::icon(FileInfoPointer info, const QIcon &feedback)
{
    QIcon icon = d->fromTheme(info->nameOf(NameInfoType::kIconName));

    if (Q_LIKELY(!icon.isNull()))
        return icon;
    icon = this->icon(info->pathOf(PathInfoType::kFilePath));

    if (Q_LIKELY(!icon.isNull()))
        return icon;

    icon = d->fromTheme(info->nameOf(NameInfoType::kGenericIconName));

    if (icon.isNull())
        icon = d->fromTheme("unknown");

    if (icon.isNull())
        return feedback;

    return icon;
}
