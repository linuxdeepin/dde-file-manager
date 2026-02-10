// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SQLITEMASTER_H
#define SQLITEMASTER_H

#include <QObject>

class SqliteMaster : public QObject
{
    Q_OBJECT

    Q_CLASSINFO("TableName", "sqlite_master")
    Q_PROPERTY(QString type READ getType WRITE setType)
    Q_PROPERTY(QString name READ getName WRITE setName)
    Q_PROPERTY(QString tbl_name READ getTbl_name WRITE setTbl_name)
    Q_PROPERTY(int rootpage READ getRootpage WRITE setRootpage)
    Q_PROPERTY(QString sql READ getSql WRITE setSql)

public:
    explicit SqliteMaster(QObject *parent = nullptr);

    QString getType() const;
    void setType(const QString &value);

    QString getName() const;
    void setName(const QString &value);

    QString getTbl_name() const;
    void setTbl_name(const QString &value);

    int getRootpage() const;
    void setRootpage(int value);

    QString getSql() const;
    void setSql(const QString &value);

private:
    QString type {};
    QString name {};
    QString tbl_name {};
    int rootpage {};
    QString sql {};
};

#endif   // SQLITEMASTER_H
