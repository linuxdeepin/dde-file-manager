/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: liuyangming<liuyangming@uniontech.com>
 *             gongheng<gongheng@uniontech.com>
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

#ifndef RECENTITERATEWORKER_H
#define RECENTITERATEWORKER_H

#include "dfmplugin_recent_global.h"

#include <QObject>

namespace dfmplugin_recent {

class RecentIterateWorker : public QObject
{
    Q_OBJECT
public:
    RecentIterateWorker();

public slots:
    void doWork();

signals:
    void updateRecentFileInfo(const QUrl &url, const QString originPath, qint64 readTime);
    void deleteExistRecentUrls(const QList<QUrl> &urls);
};
}
#endif   // RECENTITERATEWORKER_H
