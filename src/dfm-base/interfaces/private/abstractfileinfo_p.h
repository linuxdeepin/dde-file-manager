/*
 * Copyright (C) 2021 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#ifndef ABSTRACTFILEINFO_P_H
#define ABSTRACTFILEINFO_P_H

#include "interfaces/abstractfileinfo.h"
#include "utils/threadcontainer.hpp"

#include <dfm-io/dfmio_global.h>
#include <dfm-io/dfmio_register.h>
#include <dfm-io/core/diofactory.h>
#include <dfm-io/core/dfileinfo.h>

#include <QPointer>

USING_IO_NAMESPACE
namespace dfmbase {

class AbstractFileInfoPrivate
{
    friend class AbstractFileInfo;

public:
    QUrl url;   // 文件的url
    explicit AbstractFileInfoPrivate(const QUrl &url, AbstractFileInfo *qq);
    virtual ~AbstractFileInfoPrivate();

    AbstractFileInfo *const q;   // DAbstractFileInfo实例对象
    AbstractFileInfoPointer proxy { nullptr };
    QMap<AbstractFileInfo::FileExtendedInfoType, QVariant> extendOtherCache;

private:
    QUrl getUrlByChildFileName(const QString &fileName) const;
    QUrl getUrlByNewFileName(const QString &fileName) const;
    QString fileName() const;
    QString baseName() const;
    bool canDrop();
};

}

#endif   // ABSTRACTFILEINFO_P_H
