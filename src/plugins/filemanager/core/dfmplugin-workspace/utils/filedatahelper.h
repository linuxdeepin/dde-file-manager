// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
    void destroyTraversalThread(TraversalThreadPointer threadPtr);

    QMap<QUrl, RootInfo *> rootInfoMap;
    QList<TraversalThreadPointer> waitToDestroyThread;
};

}

#endif   // FILEDATAHELPER_H
