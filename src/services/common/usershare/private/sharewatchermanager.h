/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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
#ifndef SHAREWATCHERMANAGER_H
#define SHAREWATCHERMANAGER_H

#include "dfm_common_service_global.h"

#include "dfm-base/dfm_base_global.h"

#include <QObject>
#include <QMap>

namespace dfmbase {
class LocalFileWatcher;
}

namespace dfm_service_common {

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
