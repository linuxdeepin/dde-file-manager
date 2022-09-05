// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DGVFSFILEINFO_H
#define DGVFSFILEINFO_H


#include "dfileinfo.h"

class DGvfsFileInfoPrivate;
class DGvfsFileInfo : public DFileInfo
{
public:
    explicit DGvfsFileInfo(const QString &filePath, bool hasCache = true);
    explicit DGvfsFileInfo(const DUrl &fileUrl, bool hasCache = true);
    explicit DGvfsFileInfo(const DUrl &fileUrl, const QMimeType &mimetype, bool hasCache = true);
    explicit DGvfsFileInfo(const QFileInfo &fileInfo, bool hasCache = true);
    explicit DGvfsFileInfo(const QFileInfo &fileInfo, const QMimeType &mimetype, bool hasCache = true);
    ~DGvfsFileInfo() override;

    bool exists() const override;
    bool canRename() const override;
    bool isWritable() const override;
    bool makeAbsolute() override;
    bool isSymLink() const override;
    bool isDir() const override;
    QDateTime lastRead() const override;
    QDateTime lastModified() const override;
    uint ownerId() const override;
    qint64 size() const override;
    int filesCount() const override;
    QMimeType mimeType(QMimeDatabase::MatchMode mode = QMimeDatabase::MatchDefault) const override;

    void refresh(const bool isForce = false) override;
    void makeToActive() override;
    quint64 inode() const override;
    QIcon fileIcon() const override;
    QList<QIcon> additionalIcon() const override;

    void refreshCachesByStat();

    /**
     * @brief canDragCompress 是否支持拖拽压缩
     * @return
     */
    bool canDragCompress() const override;


protected:
    explicit DGvfsFileInfo(DGvfsFileInfoPrivate &dd);

private:
    Q_DECLARE_PRIVATE(DGvfsFileInfo)

    friend class RequestGvfsEP;
};
#endif // DGVFSFILEINFO_H
