/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lanxuesong<lanxuesong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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

#include "localfileiconprovider.h"

#include "dfm-base/utils/decorator/decoratorfileinfo.h"

DFMBASE_BEGIN_NAMESPACE
class LocalFileIconProviderPrivate
{
public:
    LocalFileIconProviderPrivate();

    QIcon fileSystemIcon(const QString &path) const;
    QIcon fromTheme(QString iconName) const;
};
DFMBASE_END_NAMESPACE

DFMBASE_USE_NAMESPACE
LocalFileIconProviderPrivate::LocalFileIconProviderPrivate()
{
}

// fallback to gio
QIcon LocalFileIconProviderPrivate::fileSystemIcon(const QString &path) const
{
    QIcon icon;

    DecoratorFileInfo info(path);
    if (!info.exists())
        return icon;

    const QStringList &iconNames = info.standardIconNames();
    if (!iconNames.isEmpty())
        icon = QIcon::fromTheme(iconNames.first());

    return icon;
}

QIcon LocalFileIconProviderPrivate::fromTheme(QString iconName) const
{
    QIcon icon = QIcon::fromTheme(iconName);

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

QIcon LocalFileIconProvider::icon(AbstractFileInfo *info, const QIcon &feedback)
{
    QIcon icon = d->fromTheme(info->iconName());

    if (Q_LIKELY(!icon.isNull()))
        return icon;

    icon = this->icon(info->filePath());

    if (Q_LIKELY(!icon.isNull()))
        return icon;

    icon = d->fromTheme(info->genericIconName());

    if (icon.isNull())
        icon = d->fromTheme("unknown");

    if (icon.isNull())
        return feedback;

    return icon;
}
