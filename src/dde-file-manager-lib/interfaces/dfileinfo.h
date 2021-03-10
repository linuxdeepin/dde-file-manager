/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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

#include "dabstractfileinfo.h"

class DFileInfoPrivate;
class DFileInfo : public DAbstractFileInfo
{
public:
    explicit DFileInfo(const QString &filePath, bool hasCache = true);
    explicit DFileInfo(const DUrl &fileUrl, bool hasCache = true);
    explicit DFileInfo(const QFileInfo &fileInfo, bool hasCache = true);
    ~DFileInfo() override;

    static bool exists(const DUrl &fileUrl);
    static QMimeType mimeType(const QString &filePath, QMimeDatabase::MatchMode mode = QMimeDatabase::MatchDefault,
                              const QString inod = QString(), const bool isgvfs = false);

    bool exists() const override;
    bool isPrivate() const override;

    QString path() const override;
    QString filePath() const override;
    QString absolutePath() const override;
    QString absoluteFilePath() const override;
    QString fileName() const override;
    QString fileSharedName() const override;

    QList<QIcon> additionalIcon() const override;

    bool canRename() const override;
    bool canShare() const override;
    bool canFetch() const override;
    bool canTag() const override;

    bool isReadable() const override;
    bool isWritable() const override;
    bool isExecutable() const override;
    bool isHidden() const override;
    bool isRelative() const override;
    bool isAbsolute() const override;
    bool isShared() const override;
    bool isWritableShared() const override;
    bool isAllowGuestShared() const override;
    bool makeAbsolute() override;

    FileType fileType() const override;
    bool isFile() const override;
    bool isDir() const override;
    bool isSymLink() const override;

    /**
     * @brief canDragCompress 是否支持拖拽压缩
     * @return
     */
    bool canDragCompress() const override;

    QString symlinkTargetPath() const override;
    DUrl symLinkTarget() const override;

    QString owner() const override;
    uint ownerId() const override;
    QString group() const override;
    uint groupId() const override;

    bool permission(QFile::Permissions permissions) const override;
    QFile::Permissions permissions() const override;

    qint64 size() const override;
    int filesCount() const override;
    QString sizeDisplayName() const override;

    QDateTime created() const override;
    QDateTime lastModified() const override;
    QDateTime lastRead() const override;

    QMimeType mimeType(QMimeDatabase::MatchMode mode = QMimeDatabase::MatchDefault) const override;

    bool canIteratorDir() const override;

    QString subtitleForEmptyFloder() const override;

    QString fileDisplayName() const override;

    void refresh(const bool isForce = false) override;
    DUrl goToUrlWhenDeleted() const override;

    void makeToActive() override;
    void makeToInactive() override;
    QIcon fileIcon() const override;

    QString iconName() const override;

    QFileInfo toQFileInfo() const override;
    QIODevice *createIODevice() const override;

    QVariantHash extraProperties() const override;

    quint64 inode() const override;

    // 此函数高频调用，使用 DFileInfo 会降低性能
    static bool fileIsWritable(const QString &path, uint ownerId);

    /**
    * @brief isDragCompressFile 判断是否是支持拖拽压缩的文件
    * @return
    */
    bool isDragCompressFileFormat() const override;

protected:
    explicit DFileInfo(DFileInfoPrivate &dd);

private:
    Q_DECLARE_PRIVATE(DFileInfo)

    friend class RequestEP;
};

#endif // FILEINFO_H
