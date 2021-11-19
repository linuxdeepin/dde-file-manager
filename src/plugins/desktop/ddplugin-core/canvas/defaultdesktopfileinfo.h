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
#ifndef DEFAULTDESKTOPFILEINFO_H
#define DEFAULTDESKTOPFILEINFO_H
#include "dfm-base/localfile/localfileinfo.h"
#include "dfm_desktop_service_global.h"

DSB_D_BEGIN_NAMESPACE

class DefaultDesktopFileInfoPrivate;
class DefaultDesktopFileInfo : public dfmbase::LocalFileInfo
{
public:
    explicit DefaultDesktopFileInfo(const QUrl &url);
    ~DefaultDesktopFileInfo();

    QMimeType readMimeType(QMimeDatabase::MatchMode mode = QMimeDatabase::MatchDefault) const;
    QMimeType mimeType(QMimeDatabase::MatchMode mode = QMimeDatabase::MatchDefault) const;
    QIcon fromTheme(QString iconName) const;
    QIcon fileIcon() const;
    QString getIconName() const;

private:
    QScopedPointer<DefaultDesktopFileInfoPrivate> d;
};
// typedef QExplicitlySharedDataPointer<DefaultDesktopFileInfo> DFMDesktopFileInfoPointer;
typedef QSharedPointer<DefaultDesktopFileInfo> DFMDesktopFileInfoPointer;
DSB_D_END_NAMESPACE
#endif   // DEFAULTDESKTOPFILEINFO_H
