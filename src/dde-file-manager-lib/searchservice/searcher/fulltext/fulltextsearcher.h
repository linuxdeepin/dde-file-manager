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
#ifndef FULLTEXTSEARCHER_H
#define FULLTEXTSEARCHER_H

#include "abstractsearcher.h"

#include <QObject>

class FullTextSearcherPrivate;
class FullTextSearcher : public AbstractSearcher
{
    Q_OBJECT
    friend class TaskCommander;
    friend class MainController;
    friend class FullTextSearcherPrivate;

private:
    explicit FullTextSearcher(const DUrl &url, const QString &key, QObject *parent = nullptr);
    bool createIndex(const QString &path);
    bool search() override;
    void stop() override;
    bool hasItem() const override;
    QList<DUrl> takeAll() override;
    static bool isSupport(const DUrl &url);

private:
    FullTextSearcherPrivate *d = nullptr;
};

#endif   // FULLTEXTSEARCHER_H
