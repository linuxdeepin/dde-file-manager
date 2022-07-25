/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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
#ifndef ENTRYFILEINFO_P_H
#define ENTRYFILEINFO_P_H

#include "interfaces/private/abstractfileinfo_p.h"
#include "file/entry/entities/abstractentryfileentity.h"
#include "file/entry/entryfileinfo.h"

namespace dfmbase {
class EntryFileInfoPrivate : public AbstractFileInfoPrivate
{
    friend class EntryFileInfo;
    QScopedPointer<AbstractEntryFileEntity> entity { nullptr };

public:
    explicit EntryFileInfoPrivate(EntryFileInfo *qq);
    void init();
    virtual ~EntryFileInfoPrivate() override;
};

}
#endif   // ENTRYFILEINFO_P_H
