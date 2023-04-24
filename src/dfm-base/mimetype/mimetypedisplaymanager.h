// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
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
    explicit MimeTypeDisplayManager(QObject *parent = nullptr);

public:
    ~MimeTypeDisplayManager();
    static MimeTypeDisplayManager *instance();

    void initData();
    void initConnect();

    QString displayName(const QString &mimeType);
    FileInfo::FileType displayNameToEnum(const QString &mimeType);
    QString defaultIcon(const QString &mimeType);

    QMap<FileInfo::FileType, QString> displayNames();
    QStringList readlines(const QString &path);
    void loadSupportMimeTypes();
    QStringList supportArchiveMimetypes();
    QStringList supportVideoMimeTypes();
    QStringList supportAudioMimeTypes();

private:
    static MimeTypeDisplayManager *self;
    QMap<FileInfo::FileType, QString> displayNamesMap;
    QMap<FileInfo::FileType, QString> defaultIconNames;
    QStringList ArchiveMimeTypes;
    QStringList AvfsBlackList;
    QStringList TextMimeTypes;
    QStringList VideoMimeTypes;
    QStringList AudioMimeTypes;
    QStringList ImageMimeTypes;
    QStringList ExecutableMimeTypes;
    QStringList BackupMimeTypes;
};

}

#endif   // MIMETYPEDISPLAYMANAGER_H
