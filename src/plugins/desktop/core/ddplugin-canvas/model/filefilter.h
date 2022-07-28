/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#ifndef FILEFILTER_H
#define FILEFILTER_H

#include "ddplugin_canvas_global.h"

#include <QObject>
#include <QHash>

class QGSettings;

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

#endif // FILEFILTER_H
