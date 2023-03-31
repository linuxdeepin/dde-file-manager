// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SHAREWATCHERMANAGER_H
#define SHAREWATCHERMANAGER_H

#include "dfmplugin_dirshare_global.h"

#include <dfm-base/dfm_base_global.h>

#include <QObject>
#include <QMap>

namespace dfmbase {
class LocalFileWatcher;
}

namespace dfmplugin_dirshare {

class ShareWatcherManager : public QObject
{
    Q_OBJECT

public:
    explicit ShareWatcherManager(QObject *parent = nullptr);
    ~ShareWatcherManager();

    DFMBASE_NAMESPACE::LocalFileWatcher *add(const QString &path);
    void remove(const QString &path);

Q_SIGNALS:
    void fileDeleted(const QString &filePath);
    void fileAttributeChanged(const QString &filePath);
    void fileMoved(const QString &fromFilePath, const QString &toFilePath);
    void subfileCreated(const QString &filePath);

private:
    QMap<QString, DFMBASE_NAMESPACE::LocalFileWatcher *> watchers;
};

}

#endif   // SHAREWATCHERMANAGER_H
