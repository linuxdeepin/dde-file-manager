// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MIMEDATABASE_H
#define MIMEDATABASE_H

#include <dfm-base/dfm_base_global.h>

#include <QMimeDatabase>

namespace dfmbase {

class MimeDatabase
{
    Q_DISABLE_COPY(MimeDatabase)

private:
    explicit MimeDatabase();
    virtual ~MimeDatabase();

public:
    enum class FileType : uint16_t {
        kDirectory = 0,   // 目录
        kCharDevice = 1,   // 字符设备
        kBlockDevice = 2,   // 块设备
        kFIFOFile = 3,   // FIFO文件
        kSocketFile = 4,   // socket文件
        kRegularFile = 5,   // Regular文件

        kDocuments = 6,   // 文档
        kImages = 7,   // 镜像文件
        kVideos = 8,   // 视频文件
        kAudios = 9,   // 音乐文件
        kArchives = 10,   // 归档文件
        kDesktopApplication = 11,   // 应用
        kExecutable = 12,   // 可执行
        kBackups = 13,   // 回退
        kUnknown = 14,
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

}

#endif   // MIMEDATABASE_H
