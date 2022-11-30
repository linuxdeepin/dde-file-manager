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
#ifndef SHAREFILEINFO_P_H
#define SHAREFILEINFO_P_H

#include "dfmplugin_myshares_global.h"

#include "dfm-base/interfaces/private/abstractfileinfo_p.h"

namespace dfmplugin_myshares {

class ShareFileInfo;
class ShareFileInfoPrivate : public DFMBASE_NAMESPACE::AbstractFileInfoPrivate
{
    friend class ShareFileInfo;

public:
    explicit ShareFileInfoPrivate(const QUrl &url, DFMBASE_NAMESPACE::AbstractFileInfo *qq);
    virtual ~ShareFileInfoPrivate();
    void refresh();

private:
    ShareInfo info;
    QString fileName() const;
};

}

#endif   // SHAREFILEINFO_P_H
