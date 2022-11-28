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
#ifndef AVFSFILEINFO_H
#define AVFSFILEINFO_H

#include "dfmplugin_avfsbrowser_global.h"

#include "dfm-base/file/local/localfileinfo.h"

namespace dfmplugin_avfsbrowser {

class AvfsFileInfo : public DFMBASE_NAMESPACE::AbstractFileInfo
{
public:
    explicit AvfsFileInfo(const QUrl &url);
    virtual ~AvfsFileInfo() override;
};

}

#endif   // AVFSFILEINFO_H
