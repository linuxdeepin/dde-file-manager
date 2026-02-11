// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEFILTER_H
#define FILEFILTER_H

#include "ddplugin_canvas_global.h"

#include <QObject>
#include <QHash>

namespace ddplugin_canvas {

class FileFilter
{
    Q_DISABLE_COPY(FileFilter)
public:
    explicit FileFilter();
    virtual ~FileFilter();
    virtual bool fileTraversalFilter(QList<QUrl> &urls);
    virtual bool fileDeletedFilter(const QUrl &url);
    virtual bool fileCreatedFilter(const QUrl &url);
    virtual bool fileRenameFilter(const QUrl &oldUrl, const QUrl &newUrl);
    virtual bool fileUpdatedFilter(const QUrl &url);

protected:
};

class FileProvider;
class RedundantUpdateFilter : public QObject, public FileFilter
{
    Q_OBJECT
public:
    explicit RedundantUpdateFilter(FileProvider *parent);
    bool fileUpdatedFilter(const QUrl &url) override;

protected:
    void checkUpdate();
    void timerEvent(QTimerEvent *event) override;

protected:
    FileProvider *provider = nullptr;
    QHash<QUrl, int> updateList;
    int timerid = -1;
};

}

#endif   // FILEFILTER_H
