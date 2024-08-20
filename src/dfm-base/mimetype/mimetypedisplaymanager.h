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

public:
    static MimeTypeDisplayManager *instance();

    QString displayName(const QString &mimeType);
    FileInfo::FileType displayNameToEnum(const QString &mimeType);
    QString defaultIcon(const QString &mimeType);
    QMap<FileInfo::FileType, QString> displayNames();
    QStringList supportArchiveMimetypes();
    QStringList supportVideoMimeTypes();
    QStringList supportAudioMimeTypes();

private:
    explicit MimeTypeDisplayManager(QObject *parent = nullptr);
    void initData();
    void loadSupportMimeTypes();
    QStringList readlines(const QString &path);

private:
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
