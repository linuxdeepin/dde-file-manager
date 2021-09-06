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
#ifndef DMIMEDATABASE_H
#define DMIMEDATABASE_H

#include "dfm-base/dfm_base_global.h"

#include <QMimeDatabase>

DFMBASE_BEGIN_NAMESPACE
class MimeDatabase
{
    Q_DISABLE_COPY(MimeDatabase)

    //不可继承该类
private:
    explicit MimeDatabase();
    virtual ~MimeDatabase();

public:
    enum FileType {
            Directory,
            CharDevice,
            BlockDevice,
            FIFOFile,
            SocketFile,
            RegularFile,

            Documents,
            Images,
            Videos,
            Audios,
            Archives,
            DesktopApplication,
            Executable,
            Backups,
            Unknown,
            CustomType = 0x100
    };

    static QString mimeFileType(const QString &mimeFileName);

    static FileType mimeFileTypeNameToEnum(const QString &mimeFileTypeName);

    static QStringList supportMimeFileType(FileType mimeFileType);

    static QString mimeStdIcon(const QString &mimeType);

    static QMimeType mimeTypeForFile(const QString &fileName,
                                     QMimeDatabase::MatchMode mode = QMimeDatabase::MatchDefault);

    static QMimeType mimeTypeForFile(const QFileInfo &fileInfo,
                                     QMimeDatabase::MatchMode mode = QMimeDatabase::MatchDefault);

    static QList<QMimeType> mimeTypesForFileName(const QString &fileName);

    static QMimeType mimeTypeForData(const QByteArray &data);

    static QMimeType mimeTypeForData(QIODevice *device);

    static QMimeType mimeTypeForUrl(const QUrl &url);

    static QMimeType mimeTypeForFileNameAndData(const QString &fileName,
                                                QIODevice *device);

    static QMimeType mimeTypeForFileNameAndData(const QString &fileName,
                                                const QByteArray &data);

    static QString suffixForFileName(const QString &fileName);

    static QList<QMimeType> allMimeTypes();
};
DFMBASE_END_NAMESPACE
#endif // DMIMEDATABASE_H
