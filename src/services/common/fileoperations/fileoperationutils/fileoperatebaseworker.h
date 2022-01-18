/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lanxuesong<lanxuesong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             liyigang<liyigang@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#ifndef FILEOPERATEBASEWORKER_H
#define FILEOPERATEBASEWORKER_H

#include "fileoperations/fileoperationutils/abstractworker.h"
#include "dfm-base/interfaces/abstractfileinfo.h"

#include <dfm-io/core/dfile.h>

class QObject;
class QStorageInfo;

DSC_BEGIN_NAMESPACE

class FileOperateBaseWorker : public AbstractWorker
{
public:
    explicit FileOperateBaseWorker(QObject *parent = nullptr);
    virtual ~FileOperateBaseWorker() override;

public:
    AbstractJobHandler::SupportAction doHandleErrorAndWait(const QUrl &fromUrl,
                                                           const QUrl &toUrl,
                                                           const QUrl &errorUrl,
                                                           const AbstractJobHandler::JobErrorType &error,
                                                           const QString &errorMsg = QString());
    bool createFileDevices(const AbstractFileInfoPointer &fromInfo,
                           const AbstractFileInfoPointer &toInfo,
                           QSharedPointer<DFMIO::DFile> &fromeFile,
                           QSharedPointer<DFMIO::DFile> &toFile,
                           bool *result);
    bool createFileDevice(const QUrl &fromUrl,
                          const QUrl &toUrl,
                          const AbstractFileInfoPointer &needOpenInfo,
                          QSharedPointer<DFMIO::DFile> &file, bool *result);
    bool openFiles(const AbstractFileInfoPointer &fromInfo,
                   const AbstractFileInfoPointer &toInfo,
                   const QSharedPointer<DFMIO::DFile> &fromeFile,
                   const QSharedPointer<DFMIO::DFile> &toFile,
                   bool *result);
    bool openFile(const QUrl &fromUrl,
                  const QUrl &toUrl,
                  const AbstractFileInfoPointer &fileInfo,
                  const QSharedPointer<DFMIO::DFile> &file,
                  const dfmio::DFile::OpenFlags &flags,
                  bool *result);
    void setTargetPermissions(const AbstractFileInfoPointer &fromInfo,
                              const AbstractFileInfoPointer &toInfo);
    bool doReadFile(const QUrl &fromUrl,
                    const QUrl &toUrl,
                    const AbstractFileInfoPointer &fileInfo,
                    const QSharedPointer<DFMIO::DFile> &fromDevice, char *data,
                    const qint64 &blockSize,
                    qint64 &readSize,
                    bool *result);
    bool doWriteFile(const QUrl &fromUrl,
                     const QUrl &toUrl,
                     const AbstractFileInfoPointer &fileInfo,
                     const QSharedPointer<DFMIO::DFile> &toDevice,
                     const char *data,
                     const qint64 &readSize,
                     bool *result);
    void readAheadSourceFile(const AbstractFileInfoPointer &fileInfo);
    bool checkDiskSpaceAvailable(const QUrl &fromUrl,
                                 const QUrl &toUrl,
                                 QSharedPointer<QStorageInfo> targetStorageInfo,
                                 bool *result);
    bool deleteFile(const QUrl &fromUrl,
                    const QUrl &toUrl, const AbstractFileInfoPointer &fileInfo, bool *result);
    bool copyFile(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo, bool *result);
    bool copyDir(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo, bool *result);
    bool doCopyFile(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo, bool *result);
    bool doCheckFile(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo,
                     AbstractFileInfoPointer &newTargetInfo, bool *result);
    bool creatSystemLink(const AbstractFileInfoPointer &fromInfo,
                         const AbstractFileInfoPointer &toInfo,
                         bool *result);
    bool doCheckNewFile(const AbstractFileInfoPointer &fromInfo,
                        const AbstractFileInfoPointer &toInfo,
                        AbstractFileInfoPointer &newTargetInfo,
                        QString &fileNewName,
                        bool *result);
};

DSC_END_NAMESPACE

#endif   // FILEOPERATEBASEWORKER_H
