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

#ifndef DMIMEDATABASE_H
#define DMIMEDATABASE_H

#include "dfm-base/dfm_base_global.h"

#include <QMimeDatabase>

DFMBASE_BEGIN_NAMESPACE

class MimeDatabase
{
    Q_DISABLE_COPY(MimeDatabase)

private:
    explicit MimeDatabase();
    virtual ~MimeDatabase();

public:
    enum FileType {
        kDirectory,   // 目录
        kCharDevice,   // 字符设备
        kBlockDevice,   // 块设备
        kFIFOFile,   // FIFO文件
        kSocketFile,   // socket文件
        kRegularFile,   // Regular文件

        kDocuments,   // 文档
        kImages,   // 镜像文件
        kVideos,   // 视频文件
        kAudios,   // 音乐文件
        kArchives,   // 归档文件
        kDesktopApplication,   // 应用
        kExecutable,   // 可执行
        kBackups,   // 回退
        kUnknown,
        kCustomType = 0x100
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

#endif   // DMIMEDATABASE_H
