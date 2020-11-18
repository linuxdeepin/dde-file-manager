/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     liqiang <liqianga@uniontech.com>
 *
 * Maintainer: liqiang <liqianga@uniontech.com>
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

#ifndef DCUSTOMACTIONPARSER_H
#define DCUSTOMACTIONPARSER_H

#include "dcustomactiondata.h"

#include <QObject>

class DCustomActionParser : public QObject
{
    Q_OBJECT
public:
    explicit DCustomActionParser(QObject *parent = nullptr);

    //加载文件夹遍历文件
    bool loadDir(const QString &dirPath);

    //获取遍历后的菜单配置信息
    QList<DCustomActionEntry> getActionFiles();
signals:

public slots:
private:
    QList<DCustomActionEntry> m_actionFiles;
};

#endif // DCUSTOMACTIONPARSER_H
