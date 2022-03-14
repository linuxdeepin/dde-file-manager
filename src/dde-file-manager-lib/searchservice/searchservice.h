/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
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
#ifndef SEARCHSERVICE_H
#define SEARCHSERVICE_H

#include "durl.h"

#include <QObject>

#define searchServ SearchService::instance()

class SearchServicePrivate;
class SearchService : public QObject
{
    Q_OBJECT
public:
    static SearchService *instance();

    bool search(const QString &taskId, const DUrl &url, const QString &keyword);
    QList<DUrl> matchedResults(const QString &taskId);
    void stop(const QString &taskId);
    void createFullTextIndex();

signals:
    void matched(const QString &taskId);
    void searchCompleted(const QString &taskId);
    void searchStoped(const QString &taskId);

private:
    explicit SearchService(QObject *parent = nullptr);
    ~SearchService();
    void init();

    QScopedPointer<SearchServicePrivate> d;
};

#endif   // SEARCHSERVICE_H
