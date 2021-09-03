/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liyigang<liyigang@uniontech.com>
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
    Q_DECLARE_PUBLIC(LocalMenu)
public:
    LocalMenuPrivate(const QString &filePath, LocalMenu *qq);
    virtual~LocalMenuPrivate();

public:
    QSharedPointer<LocalFileInfo> m_fileInfo {nullptr};
    bool m_isAddOemExternalAction { false };
    bool m_isNeedLoadCustomActions { false };
    LocalMenu *q_ptr {nullptr};

};
DFMBASE_END_NAMESPACE

#endif // LOCALMENU_P_H
