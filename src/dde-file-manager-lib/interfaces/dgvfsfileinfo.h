/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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
    QMimeType mimeType(QMimeDatabase::MatchMode mode = QMimeDatabase::MatchExtension) const override;

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
