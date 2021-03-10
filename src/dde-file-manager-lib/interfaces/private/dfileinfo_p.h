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

#ifndef DFILEINFO_P_H
#define DFILEINFO_P_H

#include "dabstractfileinfo_p.h"

#include <QFileInfo>
#include <QIcon>
#include <QTimer>
#include <QFuture>
#include <QQueue>

class DFileInfo;
class RequestEP;
class DFileInfoPrivate : public DAbstractFileInfoPrivate
{
public:
    DFileInfoPrivate(const DUrl &fileUrl, DFileInfo *qq, bool hasCache = true);
    ~DFileInfoPrivate();

    bool isLowSpeedFile() const;

    QFileInfo fileInfo;
    mutable QMimeType mimeType;
    mutable QMimeDatabase::MatchMode mimeTypeMode;
    mutable QIcon icon;
    mutable bool iconFromTheme = false;
    mutable QPointer<QTimer> getIconTimer;
    bool requestingThumbnail = false;
    mutable bool needThumbnail = false;
    // 小于0时表示此值未初始化，0表示不支持，1表示支持
    mutable qint8 hasThumbnail = -1;
    mutable qint8 lowSpeedFile = -1;
    mutable quint64 inode = 0;

    mutable QVariantHash extraProperties;
    mutable bool epInitialized = false;
    mutable QPointer<QTimer> getEPTimer;
    mutable QPointer<RequestEP> requestEP;
};

Q_DECLARE_METATYPE(DFileInfoPrivate*)

#endif // DFILEINFO_P_H
