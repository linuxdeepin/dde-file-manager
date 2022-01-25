/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lanxuesong<lanxuesong@uniontech.com>
 *
 * Maintainer: liyigang<liyigang@uniontech.com>
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

#ifndef MIMETYPEDISPLAYMANAGER_H
#define MIMETYPEDISPLAYMANAGER_H

#include "interfaces/abstractfileinfo.h"

#include <QObject>
#include <QMap>

DFMBASE_BEGIN_NAMESPACE

class MimeTypeDisplayManager : public QObject
{
    Q_OBJECT
    explicit MimeTypeDisplayManager(QObject *parent = nullptr);

public:
    typedef AbstractFileInfo::Type FileType;

    ~MimeTypeDisplayManager();

    void initData();
    void initConnect();

    QString displayName(const QString &mimeType);
    FileType displayNameToEnum(const QString &mimeType);
    QString defaultIcon(const QString &mimeType);

    QMap<FileType, QString> displayNames();
    static QStringList readlines(const QString &path);
    static void loadSupportMimeTypes();
    static QStringList supportArchiveMimetypes();
    static QStringList supportVideoMimeTypes();
    static MimeTypeDisplayManager *instance();

private:
    static MimeTypeDisplayManager *self;
    QMap<FileType, QString> displayNamesMap;
    QMap<FileType, QString> defaultIconNames;
    static QStringList ArchiveMimeTypes;
    static QStringList AvfsBlackList;
    static QStringList TextMimeTypes;
    static QStringList VideoMimeTypes;
    static QStringList AudioMimeTypes;
    static QStringList ImageMimeTypes;
    static QStringList ExecutableMimeTypes;
    static QStringList BackupMimeTypes;
};

DFMBASE_END_NAMESPACE

#endif   // MIMETYPEDISPLAYMANAGER_H
