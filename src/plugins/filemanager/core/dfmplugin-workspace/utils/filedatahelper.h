/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             max-lv<lvwujun@uniontech.com>
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
#ifndef FILEDATAHELPER_H
#define FILEDATAHELPER_H

#include "dfmplugin_workspace_global.h"
#include "models/fileitemdata.h"
#include "models/rootinfo.h"
#include "utils/filedatacachethread.h"

#include "dfm-base/dfm_base_global.h"

namespace dfmbase {
class AbstractFileWatcher;
}

namespace dfmplugin_workspace {

class FileViewModel;
class FileDataHelper : public QObject
{
    Q_OBJECT
public:
    explicit FileDataHelper(QObject *parent = nullptr);

    int preSetRoot(const QUrl &rootUrl);
    RootInfo *setRoot(const QUrl &rootUrl);
    RootInfo *findRootInfo(const QUrl &url);
    RootInfo *findRootInfo(const int rowIndex);
    FileItemData *findFileItemData(const int rootIndex, const int childIndex);
    QPair<int, int> getChildIndexByUrl(const QUrl &rootUrl, const QUrl &url);

    int rootsCount();
    int filesCount(const int rootIndex);

    void doTravers(const int rootIndex);
    void doStopWork(const QUrl &rootUrl);

    void update(const QUrl &rootUrl);
    void clear(const QUrl &rootUrl);

    void setFileActive(const int rootIndex, const int childIndex, bool active);

public slots:
    void updateRootInfoStatus(const QString &, const QString &mountPoint);

private:
    FileViewModel *model() const;

    RootInfo *createRootInfo(const QUrl &url);
    AbstractFileWatcherPointer setWatcher(const QUrl &url);
    TraversalThreadPointer setTraversalThread(RootInfo *info);

    QMap<QUrl, RootInfo *> rootInfoMap;
};

}

#endif   // FILEDATAHELPER_H
