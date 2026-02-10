// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROXYFILEINFO_H
#define PROXYFILEINFO_H

#include <dfm-base/interfaces/fileinfo.h>

namespace dfmbase {
class ProxyFileInfo : public FileInfo
{
public:
    explicit ProxyFileInfo(const QUrl &url);
    ~ProxyFileInfo() override;
    virtual QUrl fileUrl() const override;
    virtual bool exists() const override;
    virtual void refresh() override;

protected:
    void setProxy(const FileInfoPointer &proxy);
    void setNotifyUrl(const QUrl &url, const QString &token);
    void removeNotifyUrl(const QUrl &url, const QString &token);
    FileInfoPointer proxy { nullptr };

    // AbstractFileInfo interface
public:
    virtual QString filePath() const override;
    virtual QString absoluteFilePath() const override;
    virtual QString fileName() const override;
    virtual QString baseName() const override;
    virtual QString completeBaseName() const override;
    virtual QString suffix() const override;
    virtual QString completeSuffix() const override;
    virtual QString path() const override;
    virtual QString absolutePath() const override;
    virtual bool isReadable() const override;
    virtual bool isWritable() const override;
    virtual bool isExecutable() const override;
    virtual bool isHidden() const override;
    virtual bool isNativePath() const override;
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
    virtual bool permission(QFileDevice::Permissions permissions) const override;
    virtual QFileDevice::Permissions permissions() const override;
    virtual int countChildFile() const override;
    virtual qint64 size() const override;
    virtual quint32 birthTime() const override;
    virtual quint32 metadataChangeTime() const override;
    virtual quint32 lastModified() const override;
    virtual quint32 lastRead() const override;

    // FileInfo interface
public:
    virtual bool initQuerier() override;
    virtual void initQuerierAsync(int ioPriority, initQuerierAsyncCallback func, void *userData) override;
    virtual void cacheAttribute(dfmio::DFileInfo::AttributeID id, const QVariant &value) override;
    virtual QString nameOf(const FileNameInfoType type) const override;
    virtual QString pathOf(const FilePathInfoType type) const override;
    virtual QString displayOf(const DisplayInfoType type) const override;
    virtual QUrl urlOf(const FileUrlInfoType type) const override;
    virtual QUrl getUrlByType(const FileUrlInfoType type, const QString &fileName) const override;
    virtual bool isAttributes(const FileIsType type) const override;
    virtual bool canAttributes(const FileCanType type) const override;
    virtual QVariant extendAttributes(const FileExtendedInfoType type) const override;
    virtual int countChildFileAsync() const override;
    virtual QVariant timeOf(const FileTimeType type) const override;
    virtual QIcon fileIcon() override;
    virtual QMimeType fileMimeType(QMimeDatabase::MatchMode mode = QMimeDatabase::MatchDefault) override;
    virtual QMimeType fileMimeTypeAsync(QMimeDatabase::MatchMode mode = QMimeDatabase::MatchDefault) override;
    virtual QVariantHash extraProperties() const override;
    virtual QVariant customData(int role) const override;
    virtual FileType fileType() const override;
    virtual Qt::DropActions supportedOfAttributes(const SupportType type) const override;
    virtual QString viewOfTip(const ViewType type) const override;
    virtual QVariant customAttribute(const char *key, const dfmio::DFileInfo::DFileAttributeType type) override;
    virtual QMap<dfmio::DFileInfo::AttributeExtendID, QVariant> mediaInfoAttributes(dfmio::DFileInfo::MediaType type, QList<dfmio::DFileInfo::AttributeExtendID> ids) const override;
    virtual void setExtendedAttributes(const FileExtendedInfoType &key, const QVariant &value) override;
    virtual void updateAttributes(const QList<FileInfoAttributeID> &types = {}) override;
};
}
#endif   // PROXYFILEINFO_H
