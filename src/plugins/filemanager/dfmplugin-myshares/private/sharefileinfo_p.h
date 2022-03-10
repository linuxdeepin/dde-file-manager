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

#include "services/common/usershare/usershare_defines.h"
#include "services/common/usershare/usershareservice.h"
#include "dfm-base/interfaces/private/abstractfileinfo_p.h"

DPMYSHARES_BEGIN_NAMESPACE

class ShareFileInfo;
class ShareFileInfoPrivate : public dfmbase::AbstractFileInfoPrivate
{
    friend class ShareFileInfo;

public:
    explicit ShareFileInfoPrivate(const QUrl &url, dfmbase::AbstractFileInfo *qq);

    virtual ~ShareFileInfoPrivate();

private:
    DSC_NAMESPACE::ShareInfo info;
};

DPMYSHARES_END_NAMESPACE

#endif   // SHAREFILEINFO_P_H
