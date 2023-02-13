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
    static QStringList readlines(const QString &path);
    static void loadSupportMimeTypes();
    static QStringList supportArchiveMimetypes();
    static QStringList supportVideoMimeTypes();
    static MimeTypeDisplayManager *instance();
    static QStringList supportAudioMimeTypes();

private:
    static MimeTypeDisplayManager *self;
    QMap<AbstractFileInfo::FileType, QString> displayNamesMap;
    QMap<AbstractFileInfo::FileType, QString> defaultIconNames;
    static QStringList ArchiveMimeTypes;
    static QStringList AvfsBlackList;
    static QStringList TextMimeTypes;
    static QStringList VideoMimeTypes;
    static QStringList AudioMimeTypes;
    static QStringList ImageMimeTypes;
    static QStringList ExecutableMimeTypes;
    static QStringList BackupMimeTypes;
};

}

#endif   // MIMETYPEDISPLAYMANAGER_H
