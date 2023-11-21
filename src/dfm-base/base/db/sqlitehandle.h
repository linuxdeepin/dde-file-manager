// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SQLITEHANDLE_H
#define SQLITEHANDLE_H

#include <dfm-base/dfm_base_global.h>
#include <dfm-base/base/db/sqlitehelper.h>
#include <dfm-base/base/db/sqlitequeryable.h>

#include <QObject>
#include <QDebug>

DFMBASE_BEGIN_NAMESPACE

class SqliteHandle
{
public:
    explicit SqliteHandle(const QString &database)
        : databaseName(database) {}
    ~SqliteHandle() {}

    inline bool transaction(std::function<bool()> func)
    {
        Q_ASSERT(func);
        QSqlDatabase db { SqliteConnectionPool::instance().openConnection(databaseName) };
        db.transaction();
        if (func())
            return db.commit();

        return db.rollback();
    }

    // Create table
    template<typename T, typename... Args>
    bool createTable(const Args &... constraints)
    {
        static_assert(std::is_base_of<QObject, T>::value, "Template type T must be derived QObject");
        const QStringList &fieldNames { SqliteHelper::fieldNames<T>() };
        if (fieldNames.isEmpty()) {
            qCWarning(logDFMBase) << "Empty fields!";
            return false;
        }

        QHash<QString, QString> fieldFixes;
        SqliteHelper::fieldTypesMap<T>(fieldNames, &fieldFixes);
        if (fieldFixes.size() != fieldNames.size()) {
            qCWarning(logDFMBase) << "field types is not matched";
            return false;
        }

        QString tableFixes;
        SqliteHelper::parseConstraint(&tableFixes, &fieldFixes, constraints...);

        QString fmt;
        std::for_each(fieldNames.constBegin(), fieldNames.constEnd(), [&fmt, &fieldFixes](const QString &field) {
            fmt += (field + fieldFixes[field] + ",");
        });
        fmt += std::move(tableFixes);
        if (fmt.endsWith(","))
            fmt.chop(1);

        return excute("CREATE TABLE IF NOT EXISTS " + SqliteHelper::tableName<T>()
                      + "(" + fmt + ");");
    }

    // Drop table
    template<typename T>
    bool dropTable()
    {
        static_assert(std::is_base_of<QObject, T>::value, "Template type T must be derived QObject");
        return excute("DROP TABLE " + SqliteHelper::tableName<T>() + ";");
    }

    // Insert
    template<typename T>
    int insert(const T &entity, bool customPK = false)
    {
        static_assert(std::is_base_of<QObject, T>::value, "Template type T must be derived QObject");
        const QStringList &fieldNames { SqliteHelper::fieldNames<T>() };
        Q_ASSERT(!fieldNames.isEmpty());

        QString fmtFields;
        QString fmtValues;

        auto serializeValue = [&entity](const QString &field) -> QString {
            QVariant &&variant { entity.property(field.toLocal8Bit().data()) };
            QString &&typeStr { SqliteHelper::typeString(variant.type()) };
            QString out;
            if (typeStr.contains("TEXT")) {
                SerializationHelper::serialize(&out, variant.toString());
            } else {
                SerializationHelper::serialize(&out, variant);
            }
            return out;
        };

        int startIndex { 1 };
        if (customPK)
            startIndex = 0;

        for (int i = startIndex; i != fieldNames.size(); ++i) {
            fmtFields += (fieldNames[i] + ",");
            fmtValues += (serializeValue(fieldNames[i]) + ",");
        }

        if (fmtFields.endsWith(","))
            fmtFields.chop(1);
        if (fmtValues.endsWith(","))
            fmtValues.chop(1);

        Q_ASSERT(!fmtFields.isEmpty() && !fmtValues.isEmpty());
        int lastId { -1 };
        if (!excute("INSERT INTO " + SqliteHelper::tableName<T>()
                            + "(" + fmtFields + ") VALUES (" + fmtValues + ");",
                    [&lastId](QSqlQuery *query) {
                        Q_ASSERT(query);
                        lastId = query->lastInsertId().toInt();
                    }))
            return -1;

        return lastId;
    }

    // U: Update
    template<typename T>
    bool update(const Expression::SetExpr &setExpr, const Expression::Expr &whereExpr)
    {
        static_assert(std::is_base_of<QObject, T>::value, "Template type T must be derived QObject");
        return excute("UPDATE " + SqliteHelper::tableName<T>()
                      + " SET " + setExpr.toString()
                      + " WHERE " + whereExpr.toString());
    }

    // R: Query
    template<typename T>
    SqliteQueryable<T> query()
    {
        static_assert(std::is_base_of<QObject, T>::value, "Template type T must be derived QObject");
        return SqliteQueryable<T>(databaseName, " FROM " + SqliteHelper::tableName<T>());
    }

    // Delete
    template<typename T>
    bool remove(const T &entity)
    {
        static_assert(std::is_base_of<QObject, T>::value, "Template type T must be derived QObject");
        const QStringList &fieldNames { SqliteHelper::fieldNames<T>() };
        Q_ASSERT(!fieldNames.isEmpty());

        // use first key (ususally is Primary Key) as where expression
        auto &&field { fieldNames[0] };
        Expression::Expr &&expr { field, "=", entity.property(field.toLocal8Bit().data()) };
        return remove<T>(expr);
    }

    // Delete
    template<typename T>
    bool remove(const Expression::Expr &whereExpr)
    {
        static_assert(std::is_base_of<QObject, T>::value, "Template type T must be derived QObject");

        return excute("DELETE FROM " + SqliteHelper::tableName<T>()
                      + " WHERE " + whereExpr.toString() + ";");
    }

    inline bool excute(const QString &sql, std::function<void(QSqlQuery *)> fn = nullptr)
    {
        return SqliteHelper::excute(databaseName, sql, &lastExcutedSql, fn);
    }

    inline QString lastQuery() const
    {
        return lastExcutedSql;
    }

private:
    QString databaseName;
    QString lastExcutedSql;
};

DFMBASE_END_NAMESPACE

#endif   // SQLITEHANDLE_H
