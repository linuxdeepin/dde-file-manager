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
#ifndef TAGFILEINFO_P_H
#define TAGFILEINFO_P_H

#include "dfmplugin_tag_global.h"

#include "dfm-base/interfaces/private/abstractfileinfo_p.h"

namespace dfmplugin_tag {

class TagFileInfo;
class TagFileInfoPrivate : public DFMBASE_NAMESPACE::AbstractFileInfoPrivate
{
    friend class TagFileInfo;

public:
    explicit TagFileInfoPrivate(const QUrl &url, DFMBASE_NAMESPACE::AbstractFileInfo *qq);
    virtual ~TagFileInfoPrivate();

private:
    QString fileName() const;
};

}

#endif   // TAGFILEINFO_P_H
