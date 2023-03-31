// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SQLITEQUERYABLEH
#define SQLITEQUERYABLEH

#include <dfm-base/dfm_base_global.h>
#include <dfm-base/base/db/sqlitehelper.h>

#include <QString>
#include <QList>
#include <QSharedPointer>

// SELECT [DISTINCT] ...
// FROM TABLE
//      [LEFT] JOIN TABLE ON ...
//      ...
// WHERE ...
// GROUP BY <field>
// HAVING ...
// [<Compound> SELECT ... FROM ...]
// ORDER BY <field> [DESC], ...
// LIMIT <take> OFFSET <skip>;

DFMBASE_BEGIN_NAMESPACE

template<typename T>
class SqliteQueryable
{
public:
    SqliteQueryable(const QString &dbName,
                    const QString &from,
                    const QString &select = "SELECT ",
                    const QString &target = "*",
                    const QString &where = "")
        : databaseName(dbName),
          sqlFrom(from),
          sqlSelect(select),
          sqlTarget(target),
          sqlWhere(where)
    {
    }

    inline SqliteQueryable<T> &disctinct()
    {
        sqlSelect = "SELECT DISTINCT ";
        return *this;
    }

    inline SqliteQueryable<T> &where(const Expression::Expr &whereExpr)
    {
        sqlWhere = " WHERE " + whereExpr.toString();
        return *this;
    }

    inline SqliteQueryable<T> &groupBy(const Expression::ExprField &field)
    {
        sqlGroupBy = " GROUP BY " + field.fieldName;
        return *this;
    }

    inline SqliteQueryable<T> &having(const Expression::Expr &expr)
    {
        sqlHaving = " HAVING " + expr.toString();
        return *this;
    }

    inline SqliteQueryable<T> &take(size_t count)
    {
        sqlLimit = " LIMIT " + QString::number(count);
        return *this;
    }

    inline SqliteQueryable<T> &skip(size_t count)
    {
        if (sqlLimit.isEmpty())
            sqlLimit = " LIMIT ~0";   // ~0 is a trick :-)
        sqlOffset = " OFFSET " + QString::number(count);
        return *this;
    }

    inline SqliteQueryable<T> &orderBy(const Expression::ExprField &field)
    {
        if (sqlOrderBy.isEmpty())
            sqlOrderBy = " ORDER BY " + field.fieldName;
        else
            sqlOrderBy += "," + field.fieldName;
        return *this;
    }

    inline SqliteQueryable<T> &orderByDescending(const Expression::ExprField &field)
    {
        if (sqlOrderBy.isEmpty())
            sqlOrderBy = " ORDER BY " + field.fieldName + " DESC";
        else
            sqlOrderBy += "," + field.fieldName + " DESC";
        return *this;
    }

    inline QSharedPointer<T> toBean() const
    {
        auto &&beans { toBeans() };
        return beans.size() > 0 ? beans.first() : nullptr;
    }

    inline QList<QSharedPointer<T>> toBeans() const
    {
        QList<QSharedPointer<T>> ret;
        const QList<QVariantMap> &maps = toMaps();
        std::for_each(maps.begin(), maps.end(), [&ret](const QVariantMap &map) {
            ret.push_back(QSharedPointer<T> { SerializationHelper::deserialize<T>(map) });
        });

        return ret;
    }

    inline QVariantMap toMap() const
    {
        auto &&maps { toMaps() };
        return maps.size() > 0 ? maps.first() : QVariantMap {};
    }

    inline QList<QVariantMap> toMaps() const
    {
        const QString &sql { sqlSelect + sqlTarget + getFromSql() + getLimit() + ";" };
        QString lastQuery;
        QList<QVariantMap> maps;
        SqliteHelper::excute(databaseName, sql, &lastQuery, [&maps](QSqlQuery *query) {
            Q_ASSERT(query);
            maps = SqliteQueryable::queryToMaps(query);
        });
        return maps;
    }

    inline QVariant aggregate(const Expression::Aggregate &agg) const
    {
        const QString &sql { sqlSelect + agg.fieldName + getFromSql() + getLimit() + ";" };
        QString lastQuery;
        QVariant result;

        SqliteHelper::excute(databaseName, sql, &lastQuery, [&result](QSqlQuery *query) {
            if (query->next())
                result = query->value(0);
        });

        return result;
    }

private:
    // Return FROM part for Query
    inline QString getFromSql() const
    {
        return sqlFrom + sqlWhere + sqlGroupBy + sqlHaving;
    }

    // Return ORDER BY & LIMIT part for Query
    inline QString getLimit() const
    {
        return sqlOrderBy + sqlLimit + sqlOffset;
    }

    static inline QList<QVariantMap> queryToMaps(QSqlQuery *query)
    {
        Q_ASSERT(query);
        QList<QVariantMap> rowMaps;
        const QStringList &fieldNames = SqliteHelper::fieldNames<T>();

        while (query->next()) {
            QVariantMap rowMap;

            for (const QString &fieldName : fieldNames)
                rowMap.insert(fieldName, query->value(fieldName));

            rowMaps.append(rowMap);
        }

        return rowMaps;
    }

private:
    QString databaseName;

    QString sqlFrom;
    QString sqlSelect;
    QString sqlTarget;

    QString sqlWhere;
    QString sqlGroupBy;
    QString sqlHaving;

    QString sqlOrderBy;
    QString sqlLimit;
    QString sqlOffset;
};
DFMBASE_END_NAMESPACE

#endif   // SQLITEQUERYABLEH
