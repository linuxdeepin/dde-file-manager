/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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
#ifndef VAULTFILEINFO_H
#define VAULTFILEINFO_H

#include "dfmplugin_vault_global.h"
#include "dfm-base/interfaces/abstractfileinfo.h"

namespace dfmplugin_vault {
class VaultFileInfoPrivate;
class VaultFileInfo : public DFMBASE_NAMESPACE::AbstractFileInfo
{
    Q_GADGET
    friend class VaultFileInfoPrivate;

public:
    explicit VaultFileInfo(const QUrl &url);
    virtual ~VaultFileInfo() override;

    virtual VaultFileInfo &operator=(const VaultFileInfo &fileinfo);
    virtual bool operator==(const VaultFileInfo &fileinfo) const;
    virtual bool operator!=(const VaultFileInfo &fileinfo) const;

    virtual QString pathInfo(const FilePathInfoType type = FilePathInfoType::kFilePath) const override;
    virtual QUrl urlInfo(const FileUrlInfoType type = FileUrlInfoType::kUrl) const override;
    virtual bool exists() const override;
    virtual void refresh() override;
    virtual bool isAttributes(const FileIsType type = FileIsType::kIsFile) const override;
    virtual bool canAttributes(const FileCanType type = FileCanType::kCanDrag) const override;

    virtual qint64 size() const override;
    virtual int countChildFile() const override;
    virtual QVariant extendedAttributes(const FileExtendedInfoType type = FileExtendedInfoType::kInode) const override;
    virtual QVariantHash extraProperties() const override;
    virtual QUrl getUrlByType(const FileUrlInfoType type, const QString &fileName) const override;

    virtual QIcon fileIcon() override;

    virtual QString nameInfo(const FileNameInfoType type = FileNameInfoType::kFileName) const override;

    virtual QString displayInfo(const DisplayInfoType type = DisplayInfoType::kFileDisplayName) const override;
};
}
#endif   //! VAULTFILEINFO_H
