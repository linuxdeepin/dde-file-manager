/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
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
#ifndef SEARCHFILEINFOPRIVATE_H
#define SEARCHFILEINFOPRIVATE_H
#include "dfmplugin_search_global.h"
#include "dfm-base/interfaces/private/abstractfileinfo_p.h"
namespace dfmplugin_search {
class SearchFileInfo;
class SearchFileInfoPrivate : public dfmbase::AbstractFileInfoPrivate
{
    friend class SearchFileInfo;

public:
    explicit SearchFileInfoPrivate(const QUrl &url, SearchFileInfo *qq);
    virtual ~SearchFileInfoPrivate();
    QString fileName() const;
};
}
#endif   // SEARCHFILEINFOPRIVATE_H
