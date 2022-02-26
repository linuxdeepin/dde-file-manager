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
#ifndef LOCALFILEINFOCREATOR_H
#define LOCALFILEINFOCREATOR_H

#include "interfaces/abstractfileinfocreator.h"
#include "file/local/localfileinfo.h"

#include <QReadWriteLock>

DFMBASE_BEGIN_NAMESPACE

//! LocalFileInfoCreator is to directly create variety class derived of LocalFileinfo or LocalFileInfo.
class LocalFileInfoCreator : public AbstractFileInfoCreator
{
public:
    bool registerConvertor(Convertor func, void *extData) override;
    void unregisterConvertor(Convertor func) override;
    AbstractFileInfoPointer create(const QUrl &url, bool cache = true, QString *error = nullptr) override;
    DFMLocalFileInfoPointer createLocalFileInfo(const QUrl &url, bool cache = true, QString *error = nullptr);
protected:
    //! all the function used to create variety class sould be listed here.
    static AbstractFileInfoPointer tryCovertDesktopFileInfo(const AbstractFileInfoPointer &info, void *);
protected:
    explicit LocalFileInfoCreator();
    QList<QPair<Convertor, void *>> convertFuncs;
    QReadWriteLock funcLocker;
};

DFMBASE_END_NAMESPACE

#endif // LOCALFILEINFOCREATOR_H
