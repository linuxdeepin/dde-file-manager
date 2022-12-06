/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#ifndef MASTEREDMEDIAFILEINFO_H
#define MASTEREDMEDIAFILEINFO_H

#include "dfmplugin_optical_global.h"

#include "dfm-base/interfaces/abstractfileinfo.h"

namespace dfmplugin_optical {
class MasteredMediaFileInfo : public DFMBASE_NAMESPACE::AbstractFileInfo
{
    Q_GADGET

public:
    explicit MasteredMediaFileInfo(const QUrl &url);

    bool exists() const override;
    virtual QString displayInfo(const DisplayInfoType type = DisplayInfoType::kFileDisplayName) const override;
    QString nameInfo(const FileNameInfoType type = FileNameInfoType::kFileName) const override;
    virtual QUrl urlInfo(const FileUrlInfoType type = FileUrlInfoType::kUrl) const override;
    virtual bool isAttributes(const FileIsType type = FileIsType::kIsFile) const override;
    QVariantHash extraProperties() const override;

    void refresh() override;
    virtual bool canAttributes(const FileCanType type = FileCanType::kCanDrag) const override;
    Qt::DropActions supportedAttributes(const SupportType type = SupportType::kDrag) const override;
    QString viewTip(const ViewType type = ViewType::kEmptyDir) const override;
};

}

#endif   // MASTEREDMEDIAFILEINFO_H
