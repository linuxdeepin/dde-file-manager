// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEINFOHELPER_H
#define FILEINFOHELPER_H

#include <dfm-base/dfm_base_global.h>
#include <dfm-base/utils/fileinfoasycworker.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/utils/threadcontainer.h>

#include <dfm-io/dfileinfo.h>

#include <QObject>
#include <QThread>
#include <QVariant>
#include <QMimeDatabase>
#include <QThreadPool>
#include <QReadWriteLock>

namespace dfmbase {
class FileInfoHelper : public QObject
{
    Q_OBJECT
public:
    ~FileInfoHelper() override;
    static FileInfoHelper &instance();
    QSharedPointer<FileInfoHelperUeserData> fileCountAsync(QUrl &url);
    QSharedPointer<FileInfoHelperUeserData> fileMimeTypeAsync(const QUrl &url, const QMimeDatabase::MatchMode mode,
                                                              const QString &inod, const bool isGvfs);
    void fileRefreshAsync(const QSharedPointer<dfmbase::FileInfo> dfileInfo);
    void cacheFileInfoByThread(const QSharedPointer<FileInfo> dfileInfo);

private:
    explicit FileInfoHelper(QObject *parent = nullptr);
    void init();
    void threadHandleDfmFileInfo(const QSharedPointer<FileInfo> dfileInfo);

private:
    // send for other
Q_SIGNALS:
    void mediaDataFinished(const QUrl &sourceFile, QMap<dfmio::DFileInfo::AttributeExtendID, QVariant> properties);
    void fileCountFinished(const QUrl &url, const int fileCount);
    void fileMimeTypeFinished(const QUrl &url, const QMimeType &type);
    // shend to fileinfoasyncworker for async get attribute
    void fileCount(const QUrl &url, const QSharedPointer<FileInfoHelperUeserData> data);
    void fileMimeType(const QUrl &url, const QMimeDatabase::MatchMode mode, const QString &inod,
                      const bool isGvfs, const QSharedPointer<FileInfoHelperUeserData> data);
    void fileInfoRefresh(const QUrl &url, QSharedPointer<dfmio::DFileInfo> dfileInfo);
    // 第二个参数表示，当前是链接文件的原文件更新完成
    void fileRefreshFinished(const QUrl url, const QString &infoPtr, const bool isLinkOrg);
    void fileRefreshRequest(QSharedPointer<FileInfo> dfileInfo);
    void smbSeverMayModifyPassword(const QUrl &url);
private Q_SLOTS:
    void aboutToQuit();
    void handleFileRefresh(QSharedPointer<FileInfo> dfileInfo);

private:
    void checkInfoRefresh(QSharedPointer<FileInfo> dfileInfo);

private:
    QSharedPointer<QThread> thread { nullptr };
    QSharedPointer<FileInfoAsycWorker> worker { nullptr };
    std::atomic_bool stoped { false };
    DThreadList<FileInfoPointer> qureingInfo;
    DThreadList<FileInfoPointer> needQureingInfo;
    QThreadPool pool;
};
}

#endif   // FILEINFOHELPER_H
