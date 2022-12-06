/*
 * Copyright (C) 2021 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#ifndef FILEINFO_H
#define FILEINFO_H

#include "dfm-base/dfm_base_global.h"
#include "dfm-base/mimetype/mimedatabase.h"
#include "dfm-base/interfaces/abstractfileinfo.h"

#include <QIcon>
#include <QPointF>

namespace dfmbase {

class LocalFileInfoPrivate;
class LocalFileInfo : public AbstractFileInfo
{
    Q_OBJECT
    LocalFileInfoPrivate *d = nullptr;

public:
    enum FlagIcon {
        kWritable,   // 是否可写
        kSymLink,   // 是否是链接文件
        kReadable,   // 是否可读
    };
    Q_ENUMS(FlagIcon)

    explicit LocalFileInfo(const QUrl &url);
    LocalFileInfo(const QUrl &url, QSharedPointer<DFMIO::DFileInfo> dfileInfo);
    virtual ~LocalFileInfo() override;

    virtual bool operator==(const LocalFileInfo &fileinfo) const;
    virtual bool operator!=(const LocalFileInfo &fileinfo) const;

    virtual bool initQuerier() override;
    virtual void initQuerierAsync(int ioPriority = 0, initQuerierAsyncCallback func = nullptr, void *userData = nullptr) override;
    virtual bool exists() const override;
    virtual void refresh() override;
    virtual void refresh(DFMIO::DFileInfo::AttributeID id, const QVariant &value = QVariant()) override;
    virtual QString nameInfo(const FileNameInfoType type = FileNameInfoType::kFileName) const override;

    virtual QString pathInfo(const FilePathInfoType type = FilePathInfoType::kFilePath) const override;
    virtual QUrl urlInfo(const FileUrlInfoType type = FileUrlInfoType::kUrl) const override;
    virtual bool isAttributes(const FileIsType type = FileIsType::kIsFile) const override;
    virtual bool canAttributes(const FileCanType type = FileCanType::kCanDrag) const override;
    virtual QVariant extendedAttributes(const FileExtendedInfoType type = FileExtendedInfoType::kInode) const override;

    virtual bool permission(QFile::Permissions permissions) const override;
    virtual QFile::Permissions permissions() const override;
    virtual qint64 size() const override;

    virtual QVariant timeInfo(const FileTimeType type = FileTimeType::kCreateTime) const override;

    virtual QVariantHash extraProperties() const override;
    virtual QIcon fileIcon() override;
    virtual bool isBlockDev() const;
    virtual QString mountPath() const;
    virtual AbstractFileInfo::FileType fileType() const override;
    virtual int countChildFile() const override;

    virtual QString displayInfo(const DisplayInfoType type = DisplayInfoType::kFileDisplayName) const override;

    virtual QMimeType fileMimeType(QMimeDatabase::MatchMode mode = QMimeDatabase::MatchDefault) override;

    virtual QString viewTip(const ViewType type = ViewType::kEmptyDir) const override;

    // emblems
    virtual QVariant customAttribute(const char *key, const DFMIO::DFileInfo::DFileAttributeType type) override;

    // media info
    virtual void mediaInfoAttributes(DFMIO::DFileInfo::MediaType type, QList<DFMIO::DFileInfo::AttributeExtendID> ids) const override;

    // cache attribute
    virtual void setExtendedAttributes(const FileExtendedInfoType &key, const QVariant &value) override;

private:
    void init(const QUrl &url, QSharedPointer<DFMIO::DFileInfo> dfileInfo = nullptr);
    QMimeType mimeType(const QString &filePath, QMimeDatabase::MatchMode mode = QMimeDatabase::MatchDefault, const QString &inod = QString(), const bool isGvfs = false);
};
}
typedef QSharedPointer<DFMBASE_NAMESPACE::LocalFileInfo> DFMLocalFileInfoPointer;

#endif   // FILEINFO_H
