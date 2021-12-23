/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#ifndef STATUSBAR_P_H
#define STATUSBAR_P_H

#include <DAnchors>

#include <QObject>
#include <QString>

class StatusBarPrivate : public QObject
{
public:
    explicit StatusBarPrivate(QObject *parent = nullptr);

    QString onlyOneItemCounted;
    QString counted;
    QString onlyOneItemSelected;
    QString selected;

    QString selectFolders;
    QString selectOnlyOneFolder;
    QString selectFiles;
    QString selectOnlyOneFile;
    QString selectedNetworkOnlyOneFolder;

    int fileCount = 0;
    qint64 fileSize = 0;
    int folderCount = 0;
    int folderContains = 0;

    void initFormatStrings();
};

#endif   // STATUSBAR_P_H
