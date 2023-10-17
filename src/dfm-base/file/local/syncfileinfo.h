// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SYNCFILEINFO_H
#define SYNCFILEINFO_H

#include <dfm-base/dfm_base_global.h>
#include <dfm-base/interfaces/fileinfo.h>

#include <QIcon>
#include <QPointF>

namespace dfmbase {

class SyncFileInfoPrivate;
class SyncFileInfo : public FileInfo
{
    QSharedPointer<SyncFileInfoPrivate> d { nullptr };

public:
    enum FlagIcon {
        kWritable,   // 是否可写
        kSymLink,   // 是否是链接文件
        kReadable,   // 是否可读
    };
    Q_ENUMS(FlagIcon)

    explicit SyncFileInfo(const QUrl &url);
    SyncFileInfo(const QUrl &url, QSharedPointer<DFMIO::DFileInfo> dfileInfo);
    virtual ~SyncFileInfo() override;
    virtual bool operator==(const SyncFileInfo &fileinfo) const;
    virtual bool operator!=(const SyncFileInfo &fileinfo) const;
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
    virtual FileInfo::FileType fileType() const override;
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
    virtual void updateAttributes(const QList<FileInfoAttributeID> &types = {}) override;
};
}
typedef QSharedPointer<DFMBASE_NAMESPACE::SyncFileInfo> DFMSyncFileInfoPointer;
Q_DECLARE_METATYPE(DFMSyncFileInfoPointer)

#endif   // LOCALFILEINFO_H
