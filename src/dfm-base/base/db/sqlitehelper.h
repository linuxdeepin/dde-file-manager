// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SQLITEHELPER_H
#define SQLITEHELPER_H

#include <dfm-base/dfm_base_global.h>
#include <dfm-base/base/db/sqliteconnectionpool.h>

#include <QObject>
#include <QMetaObject>
#include <QMetaProperty>
#include <QMetaClassInfo>
#include <QSqlQuery>
#include <QVariant>
#include <QDebug>

#include <functional>

DFMBASE_BEGIN_NAMESPACE

struct SerializationHelper
{
    template<typename T>
    static inline bool serialize(QString *out, const T &value)
    {
        Q_ASSERT(out);
        QVariant v { value };

        if (!v.canConvert<QString>())
            return false;

        if (v.type() == QVariant::Type::String)
            *out = "'" + v.toString() + "'";
        else
            *out = v.toString();

        return true;
    }

    template<typename BeanType>
    [[nodiscard]] static inline BeanType *deserialize(const QVariantMap &map)
    {
        static_assert(std::is_base_of<QObject, BeanType>::value, "Template type BeanType must be derived QObject");
        BeanType *bean { new BeanType() };
        const QStringList &keys { map.keys() };
        for (const QString &key : keys) {
            const QVariant &val { map.value(key) };
            bean->setProperty(key.toLocal8Bit().data(), val);
        }

        return bean;
    }
};

namespace Expression {

// SetExpr
struct SetExpr
{
    SetExpr(const QString &fieldOpVal)
        : expr(fieldOpVal)
    {
    }

    QString toString() const
    {
        return expr;
    }

    inline SetExpr operator&&(const SetExpr &rhs) const
    {
        return SetExpr { expr + "," + rhs.expr };
    }

private:
    QString expr;
};

// Field
struct ExprField
{
    QString tableName;   // TODO: unused now
    QString fieldName;

    ExprField(const QString &table, const QString &field)
        : tableName(table), fieldName(field)
    {
    }

    inline SetExpr operator=(const QVariant &value)
    {
        QString out;
        value.type() == QVariant::Type::String ? SerializationHelper::serialize(&out, value.toString())
                                               : SerializationHelper::serialize(&out, value);
        out = fieldName + "=" + out;
        return SetExpr(out);
    }
};

struct Aggregate : public ExprField
{
    Aggregate(const QString &functioName)
        : ExprField("", functioName)
    {
    }

    Aggregate(const QString &functioName, const QString &fieldName)
        : ExprField("", functioName + "(" + fieldName + ")")
    {
    }
};

// Expr
struct Expr
{
    Expr(const QString &fieldName, const QString &op)
        : expr(fieldName + op)
    {
    }

    Expr(const ExprField &field, const QString &op)
        : Expr(field.fieldName, op)
    {
    }

    Expr(const QString &fieldName, const QString &op, const QVariant &val)
    {
        const QString &prefix { fieldName + op };
        QString suffix;
        val.type() == QVariant::Type::String ? SerializationHelper::serialize(&suffix, val.toString())
                                             : SerializationHelper::serialize(&suffix, val);
        expr = prefix + suffix;
    }

    Expr(const ExprField &field, const QString &op, const QVariant &val)
        : Expr(field.fieldName, op, val)
    {
    }

    QString toString() const
    {
        return expr;
    }

    inline Expr operator&&(const Expr &rhs) const
    {
        return andOr(rhs, " AND ");
    }
    inline Expr operator||(const Expr &rhs) const
    {
        return andOr(rhs, " OR ");
    }

private:
    Expr andOr(const Expr &rhs, const QString logOp) const
    {
        auto ret = *this;
        ret.expr = "(" + ret.expr;
        ret.expr += logOp;
        ret.expr += rhs.expr + ")";
        return ret;
    }

    QString expr;
};

// operator (==, !=, >, <, >=, <=)
inline Expr operator==(const ExprField &op, const QVariant &value)
{
    return Expr { op, "=", value };
}

inline Expr operator!=(const ExprField &op, const QVariant &value)
{
    return Expr { op, "!=", value };
}

inline Expr operator>(const ExprField &op, const QVariant &value)
{
    return Expr { op, ">", value };
}

inline Expr operator<(const ExprField &op, const QVariant &value)
{
    return Expr { op, "<", value };
}

inline Expr operator>=(const ExprField &op, const QVariant &value)
{
    return Expr { op, ">=", value };
}

inline Expr operator<=(const ExprField &op, const QVariant &value)
{
    return Expr { op, "<=", value };
}

// operator ( == nullptr, != nullptr)
inline Expr operator==(const ExprField &op, std::nullptr_t)
{
    return Expr { op, " IS NULL" };
}

inline Expr operator!=(const ExprField &op, std::nullptr_t)
{
    return Expr { op, " IS NOT NULL" };
}

// operator ( `&` -> like, '|' -> not like)
inline Expr operator&(const ExprField &op, const QString &value)
{
    return Expr { op, " LIKE ", value };
}

inline Expr operator|(const ExprField &op, const QString &value)
{
    return Expr { op, " NOT LIKE ", value };
}

template<typename T>
inline ExprField Field(const QString &fieldName)
{
    static_assert(std::is_base_of<QObject, T>::value, "Template type T must be derived QObject");
    // TODO(zhangs): check fieldName
    // TODO(zhangs): add table name
    return ExprField { "", fieldName };
}

// Aggregate Function Helpers
inline auto count()
{
    return Aggregate { "COUNT (*)" };
}

inline auto count(const QString &fieldName)
{
    return Aggregate { "COUNT", fieldName };
}

inline auto sum(const QString &fieldName)
{
    return Aggregate { "SUM", fieldName };
}

inline auto avg(const QString &fieldName)
{
    return Aggregate { "AVG", fieldName };
}

inline auto max(const QString &fieldName)
{
    return Aggregate { "MAX", fieldName };
}

inline auto min(const QString &fieldName)
{
    return Aggregate { "MIN", fieldName };
}

}   // namespace Expression

class SqliteConstraint
{
protected:
    QString constraint;
    QString field;

    SqliteConstraint(const QString &c,
                     QString f = {})
        : constraint(c), field(std::move(f))
    {
    }

    friend class SqliteHelper;

public:
    static inline SqliteConstraint primary(
            const QString &fieldName)
    {
        return SqliteConstraint { " PRIMARY KEY", fieldName };
    }

    static inline SqliteConstraint autoIncreament(
            const QString &fieldName)
    {
        return SqliteConstraint { " AUTOINCREMENT", fieldName };
    }

    static inline SqliteConstraint nullable(
            const QString &fieldName)
    {
        return SqliteConstraint { "NULLABLE", fieldName };
    }

    static inline SqliteConstraint unique(
            const QString &fieldName)
    {
        return SqliteConstraint { "UNIQUE (" + fieldName + ")" };
    }

    template<typename T>
    static inline SqliteConstraint defaultValue(
            const QString &fieldName,
            const T &value)
    {
        QString out;
        Q_ASSERT(SerializationHelper::serialize(&out, value));
        QString constraint { " DEFAULT " + out };
        return SqliteConstraint { constraint, fieldName };
    }

    static inline SqliteConstraint check(
            const Expression::Expr &expr)
    {
        return SqliteConstraint { QString { "CHECK (" + expr.toString() + ")" } };
    }
};

class SqliteHelper
{
public:
    template<typename T>
    static QString tableName()
    {
        static_assert(std::is_base_of<QObject, T>::value, "Template type T must be derived QObject");
        int count { T::staticMetaObject.classInfoCount() };
        Q_ASSERT(count > 0);
        QString name;
        for (int i = 0; i != count; ++i) {
            QMetaClassInfo &&info { T::staticMetaObject.classInfo(i) };
            if (QString(info.name()) == "TableName")
                name = info.value();
        }
        Q_ASSERT(!name.isEmpty());
        return name;
    }

    template<typename T>
    static void visit(std::function<void(const QMetaProperty &)> func)
    {
        static_assert(std::is_base_of<QObject, T>::value, "Template type T must be derived QObject");
        int count = T::staticMetaObject.propertyCount();
        for (int i = 0; i != count; ++i) {
            auto &&property = T::staticMetaObject.property(i);
            func(property);
        }
    }

    template<typename T>
    static QStringList fieldNames()
    {
        static_assert(std::is_base_of<QObject, T>::value, "Template type T must be derived QObject");
        QStringList names;
        SqliteHelper::visit<T>([&names](const QMetaProperty &property) {
            if (property.isValid())
                names.append(property.name());
        });

        // first property is `objectName`
        if (!names.isEmpty())
            names.removeFirst();

        return names;
    }

    template<typename T>
    static void fieldTypesMap(const QStringList &fields, QHash<QString, QString> *map)
    {
        static_assert(std::is_base_of<QObject, T>::value, "Template type T must be derived QObject");
        Q_ASSERT(!fields.isEmpty());
        Q_ASSERT(map);
        SqliteHelper::visit<T>([fields, map](const QMetaProperty &property) {
            if (property.isValid() && fields.contains(property.name()))
                map->insert(property.name(), SqliteHelper::typeString(property));
        });
    }

    template<typename... Args>
    static void parseConstraint(QString *, QHash<QString, QString> *)
    {
    }

    template<typename... Args>
    static void parseConstraint(QString *tablesFixes, QHash<QString, QString> *fieldFixes,
                                const SqliteConstraint &constraint, const Args &... args)
    {
        Q_ASSERT(tablesFixes);
        Q_ASSERT(fieldFixes);
        const QString &field { constraint.field };
        if (!field.isEmpty()) {
            Q_ASSERT_X(fieldFixes->contains(field), "Sqlite", "Invalid Constraint Field Name");
            if (fieldFixes->contains(field)) {
                if (constraint.constraint == "NULLABLE" || constraint.constraint.contains("PRIMARY KEY"))
                    (*fieldFixes)[field].remove(" NOT NULL");
                if (constraint.constraint != "NULLABLE")   // add constraint at field end
                    (*fieldFixes)[field] += constraint.constraint;
            }
        } else {
            if (!constraint.constraint.isEmpty())   // add constraint at table end
                tablesFixes->push_back(constraint.constraint + ",");
        }
        parseConstraint(tablesFixes, fieldFixes, args...);
    }

    static inline QString typeString(const QMetaProperty &property)
    {
        if (!property.isValid())
            return {};

        return typeString(property.type());
    }

    static inline QString typeString(QVariant::Type type)
    {
        QString typeStr;

        switch (type) {
        case QVariant::Type::Bool:
            [[fallthrough]];
        case QVariant::Type::UInt:
            [[fallthrough]];
        case QVariant::Type::LongLong:
            [[fallthrough]];
        case QVariant::Type::ULongLong:
            [[fallthrough]];
        // NOTE: there is space at front
        // use SqliteConstraint::Nullable remove `not null`
        case QVariant::Type::Int:
            typeStr = " INTEGER NOT NULL";
            break;
        case QVariant::Type::Double:
            typeStr = " REAL NOT NULL";
            break;
        case QVariant::Type::String:
            typeStr = " TEXT NOT NULL";
            break;
        default:
            typeStr = "";
        }

        assert(!typeStr.isEmpty());

        return typeStr;
    }
    static inline bool excute(const QString &databaseName, const QString &sql, QString *lastQuery = nullptr, std::function<void(QSqlQuery *)> fn = nullptr)
    {
        QSqlDatabase db { SqliteConnectionPool::instance().openConnection(databaseName) };
        QSqlQuery query { db };
        query.exec(sql);

        bool ret { true };
        if (lastQuery) {
            *lastQuery = query.lastQuery();
            qCInfo(logDFMBase).noquote() << "SQL Query:" << *lastQuery;
        }
        if (query.lastError().type() != QSqlError::NoError) {
            qCWarning(logDFMBase).noquote() << "SQL Error: " << query.lastError().text().trimmed();
            ret = false;
        }

        if (fn)
            fn(&query);

        return ret;
    }
};

DFMBASE_END_NAMESPACE

#endif   // SQLITEHELPER_H
