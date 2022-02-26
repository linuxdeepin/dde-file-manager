/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#ifndef ABSTRACTFILEINFOCREATOR_H
#define ABSTRACTFILEINFOCREATOR_H

#include "dfm-base/dfm_base_global.h"
#include "abstractfileinfo.h"
DFMBASE_BEGIN_NAMESPACE

class AbstractFileInfoCreator
{
public:
    typedef AbstractFileInfoPointer (*Convertor)(const AbstractFileInfoPointer &, void *data);
    virtual bool registerConvertor(Convertor func, void *extData) = 0;
    virtual void unregisterConvertor(Convertor func) = 0;
    virtual AbstractFileInfoPointer create(const QUrl &url, bool cache = true, QString *error = nullptr) = 0;
protected:
    explicit AbstractFileInfoCreator();
};
DFMBASE_END_NAMESPACE
#endif // ABSTRACTFILEINFOCREATOR_H
