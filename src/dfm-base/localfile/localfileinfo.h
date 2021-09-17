/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyu@uniontech.com>
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
#include "dfm-base/base/abstractfileinfo.h"

#include <QIcon>
#include <QPointF>

DFMBASE_BEGIN_NAMESPACE
class LocalFileInfoPrivate;
class LocalFileInfo : public AbstractFileInfo
{
    QSharedPointer<LocalFileInfoPrivate> d;
public:

    enum FlagIcon {
        Writable,           // 是否可写
        SymLink,            // 是否是链接文件
        Readable,           // 是否可读
    };Q_ENUMS(FlagIcon)

    enum Type {
        Directory = MimeDatabase::Directory,
        CharDevice = MimeDatabase::CharDevice,
        BlockDevice = MimeDatabase::BlockDevice,
        FIFOFile = MimeDatabase::FIFOFile,
        SocketFile = MimeDatabase::SocketFile,
        RegularFile = MimeDatabase::RegularFile,
        Documents = MimeDatabase::Documents,
        Images = MimeDatabase::Images,
        Videos = MimeDatabase::Videos,
        Audios = MimeDatabase::Audios,
        Archives = MimeDatabase::Archives,
        DesktopApplication = MimeDatabase::DesktopApplication,
        Executable = MimeDatabase::Executable,
        Backups = MimeDatabase::Backups,
        Unknown = MimeDatabase::Unknown,
        CustomType = MimeDatabase::CustomType
    };

    explicit LocalFileInfo(const QUrl &url);
    virtual ~LocalFileInfo() override;

    LocalFileInfo& operator = (const LocalFileInfo &info);
    virtual bool operator==(const LocalFileInfo &fileinfo) const;
    virtual bool operator!=(const LocalFileInfo &fileinfo) const;
    virtual void setFile(const DFMIO::DFileInfo &file) override;
    virtual bool exists() const override;
    virtual void refresh() override;
    virtual QString filePath() const override;
    virtual QString absoluteFilePath() const override;
    virtual QString fileName() const override;
    virtual QString baseName() const override;
    virtual QString completeBaseName() const override;
    virtual QString suffix() const override;
    virtual QString completeSuffix() override;
    virtual QString path() const override;
    virtual QString absolutePath() const override;
    virtual QString canonicalPath() const override;
    virtual QDir dir() const override;
    virtual QDir absoluteDir() const override;
    virtual QUrl url() const override;
    virtual bool isReadable() const override;
    virtual bool isWritable() const override;
    virtual bool isExecutable() const override;
    virtual bool isHidden() const override;
    virtual bool isFile() const override;
    virtual bool isDir() const override;
    virtual bool isSymLink() const override;
    virtual bool isRoot() const override;
    virtual bool isBundle() const override;
    virtual QString symLinkTarget() const override;
    virtual QString owner() const override;
    virtual uint ownerId() const override;
    virtual QString group() const override;
    virtual uint groupId() const override;
    virtual bool permission(QFile::Permissions permissions) const override;
    virtual QFile::Permissions permissions() const override;
    virtual qint64 size() const override;
    virtual QDateTime created() const override;
    virtual QDateTime birthTime() const override;
    virtual QDateTime metadataChangeTime() const override;
    virtual QDateTime lastModified() const override;
    virtual QDateTime lastRead() const override;
    virtual QDateTime fileTime(QFile::FileTime time) const override;

    virtual bool isBlockDev() const;
    virtual QString mountPath() const;
    virtual bool isCharDev() const;
    virtual bool isFifo() const;
    virtual bool isSocket() const;
    virtual bool isRegular() const;
    virtual Type fileType() const;
    virtual QString linkTargetPath() const;
    virtual int countChildFile() const;
    virtual QString sizeFormat() const;
    virtual QString fileDisplayName() const;
    virtual QFileInfo toQFileInfo() const;
    virtual QVariantHash extraProperties() const;
    virtual quint64 inode() const;
};
DFMBASE_END_NAMESPACE
typedef QSharedPointer<DFMBASE_NAMESPACE::LocalFileInfo> DFMLocalFileInfoPointer;

#endif // FILEINFO_H
