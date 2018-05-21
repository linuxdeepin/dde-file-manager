/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#ifndef FMSTATEMANAGER_H
#define FMSTATEMANAGER_H

#include "basemanager.h"

#include <QObject>
#include "models/fmstate.h"
#include "durl.h"


class FMStateManager : public QObject, public BaseManager
{
    Q_OBJECT

public:
    static QMap<DUrl, QPair<int, int>> SortStates;

    explicit FMStateManager(QObject *parent = 0);
    ~FMStateManager();

    void initConnect();

    static QString cacheFilePath();
    static QString sortCacheFilePath();
    static QPair<int, int> getRoleAndSortOrderByUrl(const DUrl& url);

    FMState *fmState() const;
    void setFmState(FMState *fmState);

signals:

public slots:
    void loadCache();
    void saveCache();
    static void loadSortCache();
    static void saveSortCache();
    static void cacheSortState(const DUrl& url, int role, Qt::SortOrder order);

private:

    ///###:
    static QMap<DUrl, QPair<int, int>> getUrlsForFilttering()noexcept;

    FMState *m_fmState = NULL;
};

#endif // FMSTATEMANAGER_H
