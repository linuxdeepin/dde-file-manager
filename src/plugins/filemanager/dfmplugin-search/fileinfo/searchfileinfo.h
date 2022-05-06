/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liuzhangjian@uniontech.com>
 *
 * Maintainer: liuzhangjian<liuzhangjian@uniontech.com>
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
#ifndef SEARCHFILEINFO_H
#define SEARCHFILEINFO_H

#include "dfmplugin_search_global.h"

#include "dfm-base/interfaces/abstractfileinfo.h"

DFMBASE_USE_NAMESPACE
DPSEARCH_BEGIN_NAMESPACE

class SearchFileInfoPrivate;
class SearchFileInfo : public AbstractFileInfo
{
    Q_GADGET
public:
    explicit SearchFileInfo(const QUrl &url);
    ~SearchFileInfo() override;
    virtual bool exists() const override;
    virtual bool isHidden() const override;
    virtual bool isReadable() const override;
    virtual bool isWritable() const override;
    virtual bool isDir() const override;
    virtual qint64 size() const override;

    // property for view
    virtual QString emptyDirectoryTip() const override;
    virtual QString loadingTip() const override;
};

using SearchFileInfoPointer = QSharedPointer<SearchFileInfo>;
DPSEARCH_END_NAMESPACE

#endif   // SEARCHFILEINFO_H
