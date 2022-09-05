// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFILEPROXYWATCHER_H
#define DFILEPROXYWATCHER_H

#include "dabstractfilewatcher.h"

#include <functional>

class DFileProxyWatcherPrivate;
class DFileProxyWatcher : public DAbstractFileWatcher
{
public:
    explicit DFileProxyWatcher(const DUrl &url, DAbstractFileWatcher *proxy,
                               std::function<DUrl (const DUrl &)> urlConvertFun,
                               QObject *parent = nullptr);

private slots:
    void onFileDeleted(const DUrl &url);
    void onFileAttributeChanged(const DUrl &url);
    void onFileMoved(const DUrl &fromUrl, const DUrl &toUrl);
    void onSubfileCreated(const DUrl &url);

    /**
     * @brief onFileModified 文件修改
     * @param url 文件url
     */
    void onFileModified(const DUrl &url);

private:
    Q_DECLARE_PRIVATE(DFileProxyWatcher)
};

#endif // DFILEPROXYWATCHER_H
