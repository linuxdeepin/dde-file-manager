/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 lawrence
 *
 * Author:     lawrence<hujianzhong@deepin.com>
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

#ifndef FULLTEXTSEARCH_H
#define FULLTEXTSEARCH_H

#include <QObject>
#include <dfmglobal.h>

DFM_BEGIN_NAMESPACE
class DFMFullTextSearchManager : public QObject
{
    Q_OBJECT

public:
    static DFMFullTextSearchManager *getInstance();

    /* 建立索引*/
    QStringList fulltextSearch(QString keyword);

    /*进行全文搜索*/
    int fulltextIndex(QString sourcefile);

    void clearSearchResult();
    int fulltextIndexSetUp(QString sourcefile);

private:
    explicit DFMFullTextSearchManager(QObject *parent = 0);
    bool status;
};

DFM_END_NAMESPACE
#endif // FULLTEXTSEARCH_H
