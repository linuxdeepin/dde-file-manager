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

#ifndef SEARCHHISTROYMANAGER_H
#define SEARCHHISTROYMANAGER_H

#include <QObject>

#include "basemanager.h"

class SearchHistory;

class SearchHistroyManager : public QObject, public BaseManager
{
    Q_OBJECT
public:
    explicit SearchHistroyManager(QObject *parent = 0);
    ~SearchHistroyManager();
    void load();
    void save();
    QStringList toStringList();
    static QString getSearchHistroyCachePath();
private:
    void loadJson(const QJsonObject &json);
    void writeJson(QJsonObject &json);
    QList<SearchHistory *> m_historyList;
    QStringList m_stringList;

public slots:
    void writeIntoSearchHistory(QString keyword);
};

#endif // SEARCHHISTROYMANAGER_H
