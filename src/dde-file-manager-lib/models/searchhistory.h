/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     dengkeyun<dengkeyun@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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

#ifndef SEARCHHISTORY_H
#define SEARCHHISTORY_H

#include <QObject>
#include <QDateTime>
#include "debugobejct.h"

class SearchHistory : public DebugObejct
{
    Q_OBJECT
public:
    explicit SearchHistory(QObject *parent = 0);
    SearchHistory(QDateTime time, QString keyword, QObject *parent = 0);
    QDateTime getDateTime();
    QString getKeyword();
    void setDateTime(QDateTime time);
    void setKeyword(QString keyword);
    ~SearchHistory();
private:
    QDateTime m_time;
    QString m_keyword;
signals:

public slots:
};

#endif // SEARCHHISTORY_H
