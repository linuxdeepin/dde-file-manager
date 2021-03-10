/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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

#ifndef PATHMANAGER_H
#define PATHMANAGER_H

#include <QObject>
#include <QMap>
#include <QSet>

#include "durl.h"
#include "dabstractfileinfo.h"

class PathManager : public QObject
{
    Q_OBJECT

public:
    explicit PathManager(QObject *parent = nullptr);
    ~PathManager();

    void initPaths();

    QString getSystemPath(QString key);
    QString getSystemPathDisplayName(QString key) const;
    QString getSystemPathDisplayNameByPath(QString path);
    QString getSystemPathDisplayAliasByName(const QString &name);
    QString getSystemPathIconName(QString key);
    QString getSystemPathIconNameByPath(QString path);
    QStringList getSystemDiskAndDataDiskPathGroup();
    QStringList getMountRangePathGroup();

    QMap<QString, QString> systemPathsMap() const;
    QMap<QString, QString> systemPathDisplayNamesMap() const;

    bool isSystemPath(QString path) const;
    bool isVisiblePartitionPath(const DAbstractFileInfoPointer &fi);
    bool isOptical(const DAbstractFileInfoPointer &fi);

public slots:
    void loadSystemPaths();
    void mkPath(const QString& path);

private:
    QMap<QString, QString> m_systemPathsMap;
    QMap<QString, QString> m_systemPathDisplayNamesMap;
    QMap<QString, QString> m_systemPathIconNamesMap;
    QSet<QString> m_systemPathsSet;
};

#endif // PATHMANAGER_H
