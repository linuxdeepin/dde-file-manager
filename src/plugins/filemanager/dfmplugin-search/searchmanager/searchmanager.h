/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liuzhangjian@uniontech.com>
 *
 * Maintainer: liuzhangjian<liuzhangjian@uniontech.com>
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
#ifndef SEARCHMANAGER_H
#define SEARCHMANAGER_H

#include "dfmplugin_search_global.h"

#include <QObject>
#include <QMap>

namespace dfmplugin_search {

class MainController;
class SearchManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(SearchManager)

public:
    static SearchManager *instance();

    void init();
    bool search(quint64 winId, const QString &taskId, const QUrl &url, const QString &keyword);
    QList<QUrl> matchedResults(const QString &taskId);
    void stop(const QString &taskId);
    void stop(quint64 winId);

public Q_SLOTS:
    void onIndexFullTextConfigChanged(bool enabled);

signals:
    void matched(const QString &taskId);
    void searchCompleted(const QString &taskId);
    void searchStoped(const QString &taskId);

private:
    explicit SearchManager(QObject *parent = nullptr);
    ~SearchManager();

    MainController *mainController = nullptr;
    QMap<quint64, QString> taskIdMap;
};

}

#endif   // SEARCHMANAGER_H
