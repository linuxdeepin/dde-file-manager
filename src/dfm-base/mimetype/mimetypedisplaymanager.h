// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MIMETYPEDISPLAYMANAGER_H
#define MIMETYPEDISPLAYMANAGER_H

#include "interfaces/abstractfileinfo.h"

#include <QObject>
#include <QMap>

namespace dfmbase {

class MimeTypeDisplayManager : public QObject
{
    Q_OBJECT
    explicit MimeTypeDisplayManager(QObject *parent = nullptr);

public:
    ~MimeTypeDisplayManager();

    void initData();
    void initConnect();

    QString displayName(const QString &mimeType);
    AbstractFileInfo::FileType displayNameToEnum(const QString &mimeType);
    QString defaultIcon(const QString &mimeType);

    QMap<AbstractFileInfo::FileType, QString> displayNames();
    QStringList readlines(const QString &path);
    void loadSupportMimeTypes();
    QStringList supportArchiveMimetypes();
    QStringList supportVideoMimeTypes();
    static MimeTypeDisplayManager *instance();
    QStringList supportAudioMimeTypes();

private:
    static MimeTypeDisplayManager *self;
    QMap<AbstractFileInfo::FileType, QString> displayNamesMap;
    QMap<AbstractFileInfo::FileType, QString> defaultIconNames;
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
