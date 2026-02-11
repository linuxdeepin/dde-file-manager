// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MIMETYPEDISPLAYMANAGER_H
#define MIMETYPEDISPLAYMANAGER_H

#include <dfm-base/interfaces/fileinfo.h>

#include <QObject>
#include <QMap>

namespace dfmbase {

class MimeTypeDisplayManager : public QObject
{
    Q_OBJECT

public:
    static MimeTypeDisplayManager *instance();

    QString displayName(const QString &mimeType) const;
    QString fullMimeName(const QString &mimeType) const;
    FileInfo::FileType displayNameToEnum(const QString &mimeType) const;
    QString defaultIcon(const QString &mimeType) const;
    QMap<FileInfo::FileType, QString> displayNames() const;
    QStringList supportArchiveMimetypes() const;
    QStringList supportVideoMimeTypes() const;
    QStringList supportAudioMimeTypes() const;
    QString accurateDisplayTypeFromPath(const QString &filePath) const;
    QString accurateLocalMimeTypeName(const QString &filePath) const;

private:
    explicit MimeTypeDisplayManager(QObject *parent = nullptr);
    ~MimeTypeDisplayManager();
    void initData();
    void loadSupportMimeTypes();
    QStringList readlines(const QString &path);
    QMimeType accurateLocalMimeType(const QString &filePath) const;

private:
    QMimeDatabase mimeTypeDatabase;
    QMap<FileInfo::FileType, QString> namesMap;
    QMap<FileInfo::FileType, QString> displayNamesMap;
    QMap<FileInfo::FileType, QString> defaultIconNames;
    QStringList archiveMimeTypes;
    QStringList avfsBlackList;
    QStringList textMimeTypes;
    QStringList videoMimeTypes;
    QStringList audioMimeTypes;
    QStringList imageMimeTypes;
    QStringList executableMimeTypes;
    QStringList backupMimeTypes;
};

}

#endif   // MIMETYPEDISPLAYMANAGER_H
