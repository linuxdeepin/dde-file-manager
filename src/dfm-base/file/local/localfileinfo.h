// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
    virtual void cacheAttribute(DFMIO::DFileInfo::AttributeID id, const QVariant &value = QVariant()) override;
    virtual QString nameOf(const FileNameInfoType type) const override;
    virtual QString pathOf(const FilePathInfoType type) const override;
    virtual QUrl urlOf(const FileUrlInfoType type) const override;
    virtual bool isAttributes(const FileIsType type) const override;
    virtual bool canAttributes(const FileCanType type) const override;
    virtual QVariant extendAttributes(const FileExtendedInfoType type) const override;
    virtual bool permission(QFile::Permissions permissions) const override;
    virtual QFile::Permissions permissions() const override;
    virtual qint64 size() const override;
    virtual QVariant timeOf(const FileTimeType type) const override;
    virtual QVariantHash extraProperties() const override;
    virtual QIcon fileIcon() override;
    virtual AbstractFileInfo::FileType fileType() const override;
    virtual int countChildFile() const override;
    virtual int countChildFileAsync() const override;
    virtual QString displayOf(const DisplayInfoType type) const override;
    virtual QMimeType fileMimeType(QMimeDatabase::MatchMode mode = QMimeDatabase::MatchDefault) override;
    virtual QMimeType fileMimeTypeAsync(QMimeDatabase::MatchMode mode = QMimeDatabase::MatchDefault) override;
    virtual QString viewOfTip(const ViewType type) const override;
    // emblems
    virtual QVariant customAttribute(const char *key, const DFMIO::DFileInfo::DFileAttributeType type) override;
    // media info
    virtual QMap<DFMIO::DFileInfo::AttributeExtendID, QVariant> mediaInfoAttributes(DFMIO::DFileInfo::MediaType type, QList<DFMIO::DFileInfo::AttributeExtendID> ids) const override;
    // cache attribute
    virtual void setExtendedAttributes(const FileExtendedInfoType &key, const QVariant &value) override;

private:
    void init(const QUrl &url, QSharedPointer<DFMIO::DFileInfo> dfileInfo = nullptr);
    QMimeType mimeType(const QString &filePath, QMimeDatabase::MatchMode mode = QMimeDatabase::MatchDefault, const QString &inod = QString(), const bool isGvfs = false);
};
}
typedef QSharedPointer<DFMBASE_NAMESPACE::LocalFileInfo> DFMLocalFileInfoPointer;
Q_DECLARE_METATYPE(DFMLocalFileInfoPointer)

#endif   // FILEINFO_H
