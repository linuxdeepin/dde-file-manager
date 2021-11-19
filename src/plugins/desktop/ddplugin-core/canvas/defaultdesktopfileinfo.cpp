/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liqiang<liqianga@uniontech.com>
 *
 * Maintainer: liqiang<liqianga@uniontech.com>
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
#include "private/defaultdesktopfileinfo_p.h"
#include "dfm-base/base/schemefactory.h"

#include <QDir>
#include <QIcon>
#include <QDebug>

DFMBASE_USE_NAMESPACE
DSB_D_BEGIN_NAMESPACE
DefaultDesktopFileInfo::DefaultDesktopFileInfo(const QUrl &url)
    : LocalFileInfo(url), d(new DefaultDesktopFileInfoPrivate)
{
}

DefaultDesktopFileInfo::~DefaultDesktopFileInfo()
{
}

/*!
 * 临时主题获取后续有统一接口会重新整理去掉
 */
QMimeType DefaultDesktopFileInfo::readMimeType(QMimeDatabase::MatchMode mode) const
{
    QUrl url = LocalFileInfo::url();
    if (url.isLocalFile())
        return MimeDatabase::mimeTypeForUrl(url);
    else
        return MimeDatabase::mimeTypeForFile(UrlRoute::urlToPath(url),
                                             mode);
}

/*!
 * 临时主题获取后续有统一接口会重新整理去掉
 */
QMimeType DefaultDesktopFileInfo::mimeType(QMimeDatabase::MatchMode mode) const
{
    if (!d->mimeType.isValid() || d->mimeTypeMode != mode) {

        d->mimeType = readMimeType(mode);
        d->mimeTypeMode = mode;
    }

    return d->mimeType;
}

/*!
 * 临时主题获取后续有统一接口会重新整理去掉
 */
QIcon DefaultDesktopFileInfo::fromTheme(QString iconName) const
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

/*!
 * 临时主题获取后续有统一接口会重新整理去掉
 */
QIcon DefaultDesktopFileInfo::fileIcon() const
{
    // todo: 重新规划icon的获取，此处只是临时使用
    // 1、缩略图获取，暂时不考虑缩略图了
    // 2、主题图标获取
    auto iconName = mimeType().iconName();
    QIcon icon = fromTheme(iconName);
    if (Q_LIKELY(!icon.isNull()))
        return icon;
    return QIcon();
}

QString DefaultDesktopFileInfo::getIconName() const
{
    return d->iconName;
}

DSB_D_END_NAMESPACE
