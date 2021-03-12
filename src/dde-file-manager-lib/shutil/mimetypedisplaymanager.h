/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     max-lv<lvwujun@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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

#include <QObject>
#include <QMap>

#include "dabstractfileinfo.h"

class MimeTypeDisplayManager : public QObject
{
    Q_OBJECT

public:

    typedef DAbstractFileInfo::FileType FileType;

    explicit MimeTypeDisplayManager(QObject *parent = nullptr);
    ~MimeTypeDisplayManager();

    void initData();
    void initConnect();

    QString displayName(const QString& mimeType);
    FileType displayNameToEnum(const QString& mimeType);
    QString defaultIcon(const QString& mimeType);

    QMap<FileType, QString> displayNames();
    static QStringList readlines(const QString& path);
    static void loadSupportMimeTypes();
    static QStringList supportArchiveMimetypes();
    static QStringList supportVideoMimeTypes();

private:
    QMap<FileType, QString> m_displayNames;
    QMap<FileType, QString> m_defaultIconNames;
    static QStringList ArchiveMimeTypes;
    static QStringList AvfsBlackList;
    static QStringList TextMimeTypes;
    static QStringList VideoMimeTypes;
    static QStringList AudioMimeTypes;
    static QStringList ImageMimeTypes;
    static QStringList ExecutableMimeTypes;
    static QStringList BackupMimeTypes;

    bool m_supportLoaded = false;
};

#endif // MIMETYPEDISPLAYMANAGER_H
