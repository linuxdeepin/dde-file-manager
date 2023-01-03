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
#ifndef SMBSHAREFILEINFO_H
#define SMBSHAREFILEINFO_H

#include "dfmplugin_smbbrowser_global.h"

#include "dfm-base/interfaces/abstractfileinfo.h"

namespace dfmplugin_smbbrowser {
class SmbShareFileInfoPrivate;
class SmbShareFileInfo : public dfmbase::AbstractFileInfo
{
    SmbShareFileInfoPrivate *d;

public:
    explicit SmbShareFileInfo(const QUrl &url);
    virtual ~SmbShareFileInfo() override;

    virtual QString nameOf(const FileNameInfoType type) const override;
    virtual QString displayOf(const DisplayInfoType type) const override;
    virtual QIcon fileIcon() override;
    virtual bool isAttributes(const FileIsType type) const override;
    virtual bool canAttributes(const FileCanType type) const override;
    virtual bool exists() const override;
};

}

#endif   // SMBSHAREFILEINFO_H
