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
class MasteredMediaFileInfoPrivate;
class MasteredMediaFileInfo : public DFMBASE_NAMESPACE::AbstractFileInfo
{
    Q_GADGET
    MasteredMediaFileInfoPrivate *d;

public:
    explicit MasteredMediaFileInfo(const QUrl &url);

    bool exists() const override;
    virtual QString displayOf(const DisplayInfoType type) const override;
    QString nameOf(const FileNameInfoType type) const override;
    virtual QUrl urlOf(const FileUrlInfoType type) const override;
    virtual bool isAttributes(const FileIsType type) const override;
    QVariantHash extraProperties() const override;

    void refresh() override;
    virtual bool canAttributes(const FileCanType type) const override;
    Qt::DropActions supportedOfAttributes(const SupportType type) const override;
    QString viewOfTip(const ViewType type = ViewType::kEmptyDir) const override;
};

}   // namepsace dfmplugin_optical

#endif   // MASTEREDMEDIAFILEINFO_H
