// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEINFOMODEL_P_H
#define FILEINFOMODEL_P_H

#include "fileinfomodel.h"
#include "fileprovider.h"

#include <QReadWriteLock>

namespace ddplugin_canvas {

class FileInfoModelPrivate : public QObject
{
    Q_OBJECT
public:
    enum ModelState {
        NullState = 0,
        NormalState = 0x1,
        RefreshState = 0x1 << 1
    };
    explicit FileInfoModelPrivate(FileInfoModel *qq);
    void doRefresh();
    QIcon fileIcon(FileInfoPointer info);
    void checkAndRefreshDesktopIcon(const FileInfoPointer &info, int retryCount = 5);

public slots:
    void resetData(const QList<QUrl> &urls);
    void insertData(const QUrl &url);
    void removeData(const QUrl &url);
    void replaceData(const QUrl &oldUrl, const QUrl &newUrl);
    void updateData(const QUrl &url);
    void dataUpdated(const QUrl &url, const bool isLinkOrg);
    void thumbUpdated(const QUrl &url, const QString &thumb);

public:
    QDir::Filters filters = QDir::NoFilter;
    ModelState modelState = NullState;
    FileProvider *fileProvider = nullptr;
    QList<QUrl> fileList;
    QMap<QUrl, FileInfoPointer> fileMap;
    QReadWriteLock lock;

private:
    FileInfoModel *q = nullptr;
};

}

#endif   // FILEINFOMODEL_P_H
