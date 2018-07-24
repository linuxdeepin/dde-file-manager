/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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
#include <gio/gio.h>

#include "dfileiconprovider.h"
#include "dfileinfo.h"

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
    const QByteArray &file_path = QFile::encodeName(info.absoluteFilePath());

    GFile *g_file = g_file_new_for_path(file_path);
    GError *error = nullptr;
    GFileInfo *g_file_info = g_file_query_info(g_file, G_FILE_ATTRIBUTE_STANDARD_ICON, G_FILE_QUERY_INFO_NONE, nullptr, &error);

    if (error) {
        qWarning() << error->message;

        g_error_free(error);
    } else {
        GIcon *g_icon = g_file_info_get_icon(g_file_info);
        char *g_icon_name = g_icon_to_string(g_icon);

        if (g_icon_name) {
            const QByteArray icon_name(g_icon_name);

            if (icon_name.startsWith("/")) {
                icon = QIcon(QFile::decodeName(icon_name));
            } else {
                icon = QIcon::fromTheme(QFile::decodeName(icon_name));
            }

            g_free(g_icon_name);
        }

        g_object_unref(g_file_info);
    }

    g_object_unref(g_file);

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

DFM_END_NAMESPACE
