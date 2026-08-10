// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_SQLITEBEAN_H
#define UT_SQLITEBEAN_H

#include <QObject>
#include <QString>

/**
 * @brief Minimal QObject bean used by test_sqlitequeryable.cpp to exercise the
 *        dfm-base SQLite data-access layer (SqliteHelper / SqliteQueryable /
 *        SqliteHandle). Declares a TableName classinfo and two Q_PROPERTY
 *        columns so the meta-object-driven helpers (tableName / fieldNames /
 *        fieldTypesMap / insert / remove) have something to reflect over.
 */
class UtQueryableBean : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("TableName", "ut_queryable")
    Q_PROPERTY(int id READ id WRITE setId)
    Q_PROPERTY(QString name READ name WRITE setName)
public:
    explicit UtQueryableBean(QObject *parent = nullptr)
        : QObject(parent) {}
    int id() const { return m_id; }
    void setId(int v) { m_id = v; }
    QString name() const { return m_name; }
    void setName(const QString &v) { m_name = v; }
private:
    int m_id = 0;
    QString m_name;
};

#endif   // UT_SQLITEBEAN_H
