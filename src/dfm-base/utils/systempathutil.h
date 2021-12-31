/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#ifndef SYSTEMPATHUTIL_H
#define SYSTEMPATHUTIL_H

#include "dfm-base/dfm_base_global.h"

#include <QObject>
#include <QMap>
#include <QSet>

DFMBASE_BEGIN_NAMESPACE

class SystemPathUtil final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(SystemPathUtil)

public:
    static SystemPathUtil *instance();

    QString systemPath(QString key);
    QString systemPathDisplayName(QString key) const;
    QString systemPathDisplayNameByPath(QString path);
    QString systemPathIconName(QString key) const;
    QString systemPathIconNameByPath(QString path);
    bool isSystemPath(QString path) const;

private:
    explicit SystemPathUtil(QObject *parent = nullptr);
    ~SystemPathUtil();
    void initialize();
    void mkPath(const QString &path);
    void cleanPath(QString *path) const;

public:
    void loadSystemPaths();

private:
    QMap<QString, QString> systemPathsMap;
    QMap<QString, QString> systemPathDisplayNamesMap;
    QMap<QString, QString> systemPathIconNamesMap;
    QSet<QString> systemPathsSet;
    QStringList xdgDirs;
};

DFMBASE_END_NAMESPACE
#endif   // SYSTEMPATHUTIL_H
