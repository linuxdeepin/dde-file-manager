// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
