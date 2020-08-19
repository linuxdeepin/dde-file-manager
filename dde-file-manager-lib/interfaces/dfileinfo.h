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

#ifndef FILEINFO_H
#define FILEINFO_H

#include "dabstractfileinfo.h"

class DFileInfoPrivate;
class DFileInfo : public DAbstractFileInfo
{
public:
    explicit DFileInfo(const QString& filePath, bool hasCache = true);
    explicit DFileInfo(const DUrl& fileUrl, bool hasCache = true);
    explicit DFileInfo(const QFileInfo &fileInfo, bool hasCache = true);
    ~DFileInfo();

    static bool exists(const DUrl &fileUrl);
    static QMimeType mimeType(const QString &filePath, QMimeDatabase::MatchMode mode = QMimeDatabase::MatchDefault,
                              const bool boptimise = false);

    bool exists() const Q_DECL_OVERRIDE;
    bool isPrivate() const Q_DECL_OVERRIDE;

    QString path() const Q_DECL_OVERRIDE;
    QString filePath() const Q_DECL_OVERRIDE;
    QString absolutePath() const Q_DECL_OVERRIDE;
    QString absoluteFilePath() const Q_DECL_OVERRIDE;
    QString fileName() const Q_DECL_OVERRIDE;
    QString fileSharedName() const Q_DECL_OVERRIDE;

    QList<QIcon> additionalIcon() const override;

    bool canRename() const Q_DECL_OVERRIDE;
    bool canShare() const Q_DECL_OVERRIDE;
    bool canFetch() const Q_DECL_OVERRIDE;
    bool canTag() const Q_DECL_OVERRIDE;

    bool isReadable() const Q_DECL_OVERRIDE;
    bool isWritable() const Q_DECL_OVERRIDE;
    bool isExecutable() const Q_DECL_OVERRIDE;
    bool isHidden() const Q_DECL_OVERRIDE;
    bool isRelative() const Q_DECL_OVERRIDE;
    bool isAbsolute() const Q_DECL_OVERRIDE;
    bool isShared() const Q_DECL_OVERRIDE;
    bool isWritableShared() const Q_DECL_OVERRIDE;
    bool isAllowGuestShared() const Q_DECL_OVERRIDE;
    bool makeAbsolute() Q_DECL_OVERRIDE;

    FileType fileType() const override;
    bool isFile() const Q_DECL_OVERRIDE;
    bool isDir() const Q_DECL_OVERRIDE;
    bool isSymLink() const Q_DECL_OVERRIDE;

    QString symlinkTargetPath() const override;
    DUrl symLinkTarget() const Q_DECL_OVERRIDE;

    QString owner() const Q_DECL_OVERRIDE;
    uint ownerId() const Q_DECL_OVERRIDE;
    QString group() const Q_DECL_OVERRIDE;
    uint groupId() const Q_DECL_OVERRIDE;

    bool permission(QFile::Permissions permissions) const Q_DECL_OVERRIDE;
    QFile::Permissions permissions() const Q_DECL_OVERRIDE;

    qint64 size() const Q_DECL_OVERRIDE;
    int filesCount() const Q_DECL_OVERRIDE;
    QString sizeDisplayName() const override;

    QDateTime created() const Q_DECL_OVERRIDE;
    QDateTime lastModified() const Q_DECL_OVERRIDE;
    QDateTime lastRead() const Q_DECL_OVERRIDE;

    QMimeType mimeType(QMimeDatabase::MatchMode mode = QMimeDatabase::MatchDefault) const Q_DECL_OVERRIDE;

    bool canIteratorDir() const Q_DECL_OVERRIDE;

    QString subtitleForEmptyFloder() const Q_DECL_OVERRIDE;

    QString fileDisplayName() const Q_DECL_OVERRIDE;

    void refresh() Q_DECL_OVERRIDE;
    DUrl goToUrlWhenDeleted() const Q_DECL_OVERRIDE;

    void makeToActive() Q_DECL_OVERRIDE;
    void makeToInactive() Q_DECL_OVERRIDE;
    QIcon fileIcon() const Q_DECL_OVERRIDE;

    QString iconName() const Q_DECL_OVERRIDE;

    QFileInfo toQFileInfo() const Q_DECL_OVERRIDE;
    QIODevice *createIODevice() const override;

    QVariantHash extraProperties() const Q_DECL_OVERRIDE;

    quint64 inode() const Q_DECL_OVERRIDE;

protected:
    explicit DFileInfo(DFileInfoPrivate &dd);

private:
    Q_DECLARE_PRIVATE(DFileInfo)

    friend class RequestEP;
};

#endif // FILEINFO_H
