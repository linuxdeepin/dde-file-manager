/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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

#include <dgiofile.h>
#include <dgiofileinfo.h>

#include "dfileiconprovider.h"
#include "dfileinfo.h"
#include "dgvfsfileinfo.h"

#include <QLibrary>
#include <QDebug>

DFM_BEGIN_NAMESPACE

class DFileIconProviderPrivate
{
public:
    DFileIconProviderPrivate();

    void init();
    QIcon getFilesystemIcon(const QFileInfo &info) const;
    QIcon fromTheme(QString iconName) const;
};

DFileIconProviderPrivate::DFileIconProviderPrivate()
{
    init();
}

void DFileIconProviderPrivate::init()
{

}

// fallback to gio
QIcon DFileIconProviderPrivate::getFilesystemIcon(const QFileInfo &info) const
{
    QIcon icon;
    //fix 修复当链接文件指向的文件被删时，界面卡顿。如果文件不存在，直接返回空图标
    if (!info.exists())
        return icon;
    //end

    QScopedPointer<DGioFile> file(DGioFile::createFromPath(info.absoluteFilePath()));
    //todo 当链接文件指向的文件被删除时，file->createFileInfo函数耗时很长
    QExplicitlySharedDataPointer<DGioFileInfo> fileinfo = file->createFileInfo("*", FILE_QUERY_INFO_NONE, 616);
    //end
    if (!fileinfo) {
        return icon;
    }

    QStringList icons = fileinfo->themedIconNames();
    if (!icons.isEmpty()) return QIcon::fromTheme(icons.first());

    QString iconStr(fileinfo->iconString());
    if (iconStr.startsWith('/')) {
        icon = QIcon(iconStr);
    } else {
        // blumia: I guess it will failed since the g_icon_to_string() returned str can be something like
        //         ". GThemedIcon text-plain text-x-generic text-plain-symbolic" which is definitely not
        //         a valid icon name from theme.
        //         Anyway here I keep the code as-is.
        // TODO: a more Qt way than using gio?
        icon = QIcon::fromTheme(iconStr);
    }

    return icon;
}

QIcon DFileIconProviderPrivate::fromTheme(QString iconName) const
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
    } else if (iconName == "Zoom.png"){ 
        iconName = "application-x-zoom";         
    } else {
        return icon;
    }

    icon = QIcon::fromTheme(iconName);

    return icon;
}

Q_GLOBAL_STATIC(DFileIconProvider, globalFIP)

DFileIconProvider::DFileIconProvider()
    : d_ptr(new DFileIconProviderPrivate())
{

}

DFileIconProvider::~DFileIconProvider()
{

}

DFileIconProvider *DFileIconProvider::globalProvider()
{
    return globalFIP;
}

QIcon DFileIconProvider::icon(const QFileInfo &info) const
{
    Q_D(const DFileIconProvider);

    return d->getFilesystemIcon(info);
}

QIcon DFileIconProvider::icon(const QFileInfo &info, const QIcon &feedback) const
{
    const QIcon &icon = this->icon(info);

    if (icon.isNull())
        return feedback;

    return icon;
}

QIcon DFileIconProvider::icon(const DFileInfo &info, const QIcon &feedback) const
{
    Q_D(const DFileIconProvider);

    QIcon icon = d->fromTheme(info.iconName());

    if (Q_LIKELY(!icon.isNull()))
        return icon;

    icon = this->icon(info.toQFileInfo());

    if (Q_LIKELY(!icon.isNull()))
        return icon;

    icon = d->fromTheme(info.genericIconName());

    if (icon.isNull())
        icon = d->fromTheme("unknown");

    if (icon.isNull())
        return feedback;

    return icon;
}

QIcon DFileIconProvider::icon(const DGvfsFileInfo &info, const QIcon &feedback) const
{
    Q_D(const DFileIconProvider);

    QIcon icon = d->fromTheme(info.iconName());

    if (Q_LIKELY(!icon.isNull()))
        return icon;

    icon = this->icon(info.toQFileInfo());

    if (Q_LIKELY(!icon.isNull()))
        return icon;

    icon = d->fromTheme(info.genericIconName());

    if (icon.isNull())
        icon = d->fromTheme("unknown");

    if (icon.isNull())
        return feedback;

    return icon;
}

DFM_END_NAMESPACE
