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
    explicit DGvfsFileInfo(const QString& filePath, bool hasCache = true);
    explicit DGvfsFileInfo(const DUrl& fileUrl, bool hasCache = true);
    explicit DGvfsFileInfo(const DUrl& fileUrl, const QMimeType &mimetype, bool hasCache = true);
    explicit DGvfsFileInfo(const QFileInfo &fileInfo, bool hasCache = true);
    explicit DGvfsFileInfo(const QFileInfo &fileInfo, const QMimeType &mimetype, bool hasCache = true);
    ~DGvfsFileInfo() override;

    bool exists() const Q_DECL_OVERRIDE;
    bool canRename() const Q_DECL_OVERRIDE;
    bool isWritable() const Q_DECL_OVERRIDE;
    bool makeAbsolute() Q_DECL_OVERRIDE;
    bool isSymLink() const Q_DECL_OVERRIDE;
    bool isDir() const Q_DECL_OVERRIDE;
    QDateTime lastRead() const Q_DECL_OVERRIDE;
    QDateTime lastModified() const Q_DECL_OVERRIDE;
    uint ownerId() const Q_DECL_OVERRIDE;
    qint64 size() const Q_DECL_OVERRIDE;
    int filesCount() const Q_DECL_OVERRIDE;
    QMimeType mimeType(QMimeDatabase::MatchMode mode = QMimeDatabase::MatchExtension) const Q_DECL_OVERRIDE;

    void refresh(const bool isForce = false) Q_DECL_OVERRIDE;
    void makeToActive() Q_DECL_OVERRIDE;
    quint64 inode() const Q_DECL_OVERRIDE;

    void refreshCachesByStat();

protected:
    explicit DGvfsFileInfo(DGvfsFileInfoPrivate &dd);

private:
    Q_DECLARE_PRIVATE(DGvfsFileInfo)

    friend class RequestGvfsEP;
};
#endif // DGVFSFILEINFO_H
