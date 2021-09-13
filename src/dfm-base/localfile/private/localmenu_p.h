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
#ifndef LOCALMENU_P_H
#define LOCALMENU_P_H
#include "dfm-base/dfm_base_global.h"
#include "localfile/localmenu.h"
#include "localfile/localfileinfo.h"
DFMBASE_BEGIN_NAMESPACE
class LocalMenuPrivate
{
    friend class LocalMenu;
    LocalMenu *const q;
    QSharedPointer<LocalFileInfo> localFileInfo { nullptr }; // 本地文件信息
    QAtomicInteger<bool> isAddOemExternalAction { false }; // 是否加载oem的action
    QAtomicInteger<bool> isNeedLoadCustomActions { false }; // 是否需要加载用户自定义的菜单action
public:
    explicit LocalMenuPrivate(const QString &filePath, LocalMenu *qq);
    virtual ~LocalMenuPrivate();
};
DFMBASE_END_NAMESPACE

#endif // LOCALMENU_P_H
