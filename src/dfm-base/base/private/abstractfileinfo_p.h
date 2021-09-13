/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyu@uniontech.com>
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

#include "base/abstractfileinfo.h"
#include "utils/threadcontainer.hpp"

#include <dfmio_global.h>
#include <dfmio_register.h>
#include <dfm-io/core/diofactory.h>

#include <QPointer>

#include <dfm-io/core/dfileinfo.h>

USING_IO_NAMESPACE
DFMBASE_BEGIN_NAMESPACE

namespace GlobalPrivate {
static bool dfmioIsInit = DFMIO::dfmio_init();
}

class AbstractFileInfoPrivate
{
    friend class AbstractFileInfo;
    AbstractFileInfo *const q;// DAbstractFileInfo实例对象
public:
    QUrl url;// 文件的url
    AbstractFileInfoPrivate(AbstractFileInfo *qq);
    virtual ~AbstractFileInfoPrivate();
};

DFMBASE_END_NAMESPACE

#endif // ABSTRACTFILEINFO_P_H
