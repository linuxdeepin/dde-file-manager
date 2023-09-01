// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SYSTEMPATHUTIL_H
#define SYSTEMPATHUTIL_H

#include <dfm-base/dfm_base_global.h>

#include <QObject>
#include <QMap>
#include <QSet>

namespace dfmbase {

class SystemPathUtil final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(SystemPathUtil)

public:
    static SystemPathUtil *instance();

    QString systemPath(const QString &key);
    QString systemPathOfUser(const QString &key, const QString &user) const;
    QString systemPathDisplayName(const QString &key) const;
    QString systemPathDisplayNameByPath(QString path);
    QString systemPathIconName(const QString &key) const;
    QString systemPathIconNameByPath(QString path);
    bool isSystemPath(QString path) const;
    bool checkContainsSystemPath(const QList<QUrl> &urlList);

private:
    explicit SystemPathUtil(QObject *parent = nullptr);
    ~SystemPathUtil();
    void initialize();
    void mkPath(const QString &path);
    void cleanPath(QString *path) const;
    bool checkContainsSystemPathByFileInfo(const QList<QUrl> &urlList);
    bool checkContainsSystemPathByFileUrl(const QList<QUrl> &urlList);

public:
    void loadSystemPaths();

private:
    QMap<QString, QString> systemPathsMap;
    QMap<QString, QString> systemPathDisplayNamesMap;
    QMap<QString, QString> systemPathIconNamesMap;
    QSet<QString> systemPathsSet;
    QStringList xdgDirs;
};

}
#endif   // SYSTEMPATHUTIL_H
